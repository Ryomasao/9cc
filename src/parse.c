#include "9cc.h"

// エラーを報告するための関数
// printfと同じ引数をとる
// 可変長引数については、一旦棚にあげておこう
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}


Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

// 次のトークンが期待している記号のときは、トークンを1つ読み進めて真を返す
// それ以外の場合はfalse
bool consume(char *op) {
  if(token->kind != TK_RESERVED || 
    strlen(op) != token->len ||
    memcmp(token->str, op, token->len))
    return false;
  
  token = token->next;
  return true;
}

// 次のトークンが期待している記号のときは、トークンを1つ読み進める
// それ以外の場合は、エラーを報告する
void expect(char *op) {
  if(token->kind != TK_RESERVED || 
    strlen(op) != token->len ||
    memcmp(token->str, op, token->len))
    error("'%s'ではありません", op);
  
  token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めて、その数値を返す
// それ以外の場合は、エラーを報告する
int expect_number() {
  if(token->kind != TK_NUM) 
    error("数ではありません");

  int val = token->val;
  token = token->next;
  return val;
}

// 次のトークンがidentのときはそのトークン(のアドレス)を返し、トークンを1つ読み進めておく
// それ以外の場合は、NULLを返す
Token *consume_ident(){
  if(token->kind == TK_IDENT) {
    Token *identToken = token;
    token = token->next;
    return identToken;
  }

  return NULL;
}

// グローバル変数のlocalsは変数名を格納しているリスト
// tokenに格納されている変数名がすでに存在しているかを確認する
// 存在していれば変数名のLvarを、なければNULLを返す
Lvar *find_lvar(Token *token) {
  for(Lvar *var = locals; var; var = var->next) {
    if(var->len == token->len && !memcmp(var->name, token->str, token->len)) {
      return var;
    }
  }
  return NULL;
}

// localsのリストの最後のアイテムを返す
Lvar *getLastLocalsVar() {
  Lvar *temp = locals;
  while(temp->next) 
    temp = temp->next;
  return temp;
}

bool at_eof() {
  if(token->kind == TK_EOF) 
    return true;
  return false;
}

Node *expr();

Node *term() {
  if(consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  // Tokenが変数の場合
  if(tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;

    // Tokenの変数が新しいものか、既存のものかを調べる
    Lvar *lvar = find_lvar(tok);
    if(lvar) {
      node->offset = lvar->offset;
    } else {
      // 新規の場合、Lvarをつくって、リストをつなげてく
      lvar = calloc(1, sizeof(Lvar));
      lvar->name = tok->str;
      lvar->len = tok->len;
      Lvar *prevVar = getLastLocalsVar();
      // offsetは8バイト？ずつ足してく
      lvar->offset = prevVar->offset + 8;
      node->offset = lvar->offset;
      prevVar->next = lvar;
    }

    return node;
  }

  return new_node_num(expect_number());
}

Node *unary() {
    // +3とかはただの3にする
    if(consume("+"))
      return term();

    // -3は 0 - 3のノードにする
    if(consume("-"))
      return new_node(ND_SUB, new_node_num(0), term());
    
    return term();
}

Node *mul() {
  Node *node = unary();
  for(;;) {
    if(consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if(consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();
  for(;;) {
    if(consume("+"))
      node = new_node(ND_ADD, node, mul());
    else if(consume("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();
  for(;;) {
    if(consume("<"))
      node = new_node(ND_LT, node, add());
    else if(consume("<="))
      node = new_node(ND_LTE, node, add());
    else if(consume(">"))
      node = new_node(ND_LT, add(), node);
    else if(consume(">="))
      // 左辺と右辺を入れ替えて、<= を使うようにする:
      node = new_node(ND_LTE, add(), node);
    else
      return node;
  }
}

Node *eqaulity() {
  Node *node = relational();
  for(;;) {
    if(consume("=="))
      node = new_node(ND_EQ, node, add());
    else if(consume("!="))
      node = new_node(ND_NEQ, node, add());
    else
      return node;
  }

}

Node *assign() {
  Node *node =  eqaulity();
  if(consume("=")) {
    return new_node(ND_ASSIGN, node, assign());
  }
  
  return node;
}

Node *expr() {
  return assign();
}


// statementは以下のいずれかを想定している
// return;
// if(expr) statement;
// expr;
Node *stmt() {
  Node *node;

  if(consume("return")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
  } else if(consume("if")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_IF;
    expect("(");
    node->lhs = expr();
    expect(")");
    node->rhs = stmt();
    return node;
  } else {
    node = expr();
  }

  expect(";");

  return node;
}

// code[]にはstatement;という単位で格納されていくはず
void program() {
  int i = 0;

  while(!at_eof()) 
    code[i++] = stmt();

  code[i] = NULL;
  
}

