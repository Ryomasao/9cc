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

// エラー箇所を報告するための関数
void error_at(char *loc, char*msg) {
  // 走査中の文字のアドレスからユーザー入力開始位置のアドレスを引く
  // そうすると、走査中の文字が開始位置から何文字目か判別できる
  // ASCは1文字 1byte で16進数で表現するとaは0x61
  // 61は2進数で 01100001で
  int pos = loc - user_input;

  // debug
  //printf(" address of loc  %p \n", loc);
  //printf(" address of user_input  %p \n", user_input);

  fprintf(stderr, "%s\n", user_input);
  // pos分スペースを出力する
  // そうするとこんなエラーメッセージになる素敵
  // 123a1
  //    ^ トークナイズできません
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ %s\n", msg);
  exit(1);
}


// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
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
// それ以外の場合は、NULLを返したいんだけど、NULLをどう扱っていいかわからない問題
Token *consume_ident(){
  if(token->kind == TK_IDENT) {
    Token *identToken = token;
    token = token->next;
    return identToken;
  }

  // ひとまず0を返す 当初returnを省略してたら、謎のアドレスが返されていたので。
  return 0;
}

Node *expr();

Node *term() {
  if(consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if(tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    node->offset = (tok->str[0] - 'a' + 1) * 8;
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


// 入力文字列pをトークナイズして、それを返す
Token *tokenize(char *p) {

  Token head;
  head.next = NULL;
  Token *cur = &head;

  while(*p) {
    // 空白文字はskip
    if(isspace(*p)) {
      p++;
      continue;
    }

    // > と >= は >=を優先してトークナイズする
    if(*p ==  '>' && !memcmp(p, ">=", 2)) {
      cur = new_token(TK_RESERVED, cur, p);
      cur->len = 2;
      // 2文字進める
      p += 2;
      continue;
    }

    if(*p ==  '<' && !memcmp(p, "<=", 2)) {
      cur = new_token(TK_RESERVED, cur, p);
      cur->len = 2;
      p += 2;
      continue;
    }

    if(*p ==  '=' && !memcmp(p, "==", 2)) {
      cur = new_token(TK_RESERVED, cur, p);
      cur->len = 2;
      p += 2;
      continue;
    }

    if(*p ==  '!' && !memcmp(p, "!=", 2)) {
      cur = new_token(TK_RESERVED, cur, p);
      cur->len = 2;
      p += 2;
      continue;
    }

    if(*p == '+' || 
       *p == '-' || 
       *p == '*' || 
       *p == '/' || 
       *p == '(' || 
       *p == ')' ||
       *p == '<' ||
       *p == '>' ||
       *p == '=' 
      ) 
    {
      cur = new_token(TK_RESERVED, cur, p++);
      cur->len = 1;
      continue;
    }

    if(isdigit(*p)) {
      // p++ しなくていいのかなとおもったけど、strtolが10進数以外の文字のとこまでのアドレスを
      // pに設定してくれている
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    // 変数
    if('a' <= *p && *p <='z') {
      cur = new_token(TK_IDENT, cur, p++);
      cur->len = 1;
      continue;
    }

    error_at(p, "トークナイスできません");
}

  new_token(TK_EOF, cur, p);
  return head.next;
}