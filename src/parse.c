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

// 指定されたトークンが期待している記号のときは真を返す
bool is_supposed_token(char *op, Token *targetToken) {
  if(targetToken->kind != TK_RESERVED || strlen(op) != targetToken->len ||
     memcmp(targetToken->str, op, targetToken->len))
    return false;

  return true;
}

// 次のトークンが期待している記号のときは、トークンを1つ読み進めて真を返す
bool consume(char *op) {
  if(is_supposed_token(op, token)) {
    token = token->next;
    return true;
  } else {
    return false;
  }
}

// 次のトークンが期待している記号のときは、トークンを1つ読み進める
// それ以外の場合は、エラーを報告する
void expect(char *op) {
  if(token->kind != TK_RESERVED || strlen(op) != token->len ||
     memcmp(token->str, op, token->len))
    error("'%s'ではありません。 \n%d行目あたり", op, token->line + 1);

  token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めて、その数値を返す
// それ以外の場合は、エラーを報告する
int expect_number() {
  if(token->kind != TK_NUM)
    error("'数ではありません。 \n%d行目あたり", token->line + 1);

  int val = token->val;
  token = token->next;
  return val;
}

// 次のトークンがidentのときはそのトークン(のアドレス)を返し、トークンを1つ読み進めておく
// それ以外の場合は、NULLを返す
Token *consume_ident() {
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
  for(Lvar *var = locals[functionId]; var; var = var->next) {
    if(var->len == token->len && !memcmp(var->name, token->str, token->len)) {
      return var;
    }
  }
  return NULL;
}

// localsのリストの最後のアイテムを返す
Lvar *getLastLocalsVar() {
  Lvar *temp = locals[functionId];

  // tempがNULLのとき、temp->nextもNULLでwhile(temp->next)は抜けると思ったけど
  // BAD ACCESSになるので、抜ける
  if(!temp) return NULL;

  while(temp->next)
    temp = temp->next;
  return temp;
}

bool at_eof() {
  if(token->kind == TK_EOF)
    return true;
  return false;
}

Lvar *create_or_set_lvars(Token *tok) {
  // Tokenの変数が新しいものか、既存のものかを調べる
  Lvar *lvar = find_lvar(tok);

  if(!lvar) {
    // 新しい変数の場合、Lvarをつくって、リストをつなげてく
    lvar = calloc(1, sizeof(Lvar));
    lvar->name = tok->str;
    lvar->len = tok->len;
    Lvar *prevVar = getLastLocalsVar();

    if(!prevVar) {
      // リストの先頭は、直接配列に格納する
      lvar->offset = 8;
      locals[functionId] = lvar;
    } else {
      // offsetは8バイト？ずつ足してく
      lvar->offset = prevVar->offset + 8;
      prevVar->next = lvar;
    }

  }

  return lvar;
}

// 関数の()の中をパースする
// 呼び出し後は、)←のトークンも読み進めているので、呼び出し側で)を読み飛ばす必要はないので注意
void parse_argv(Node *argv[3]) {
  int i = 0;

  while(true) {
    if(consume(")")) {
      break;
    }

    if(token->kind == TK_INT) {
      error("関数呼び出しに型情報はいらないよ");
    }

    // term()じゃなくって、lvar()を作ったほうがいいなぁ、、、
    // とおもったけど、func(n - 1)とかやるから、exprじゃなきゃだめ！
    argv[i] = expr();
    i++;

    // 最後が,じゃなかったら引数はもうないとみなす
    if(!consume(",")) {
      expect(")");
      break;
    }
  }
}

// parse_argvで型があるかをチェックする
// かなり冗長だけどひとまずね、、、
void parse_argv_with_type(Node *argv[3]) {
  int i = 0;

  while(true) {
    if(consume(")")) {
      break;
    }

    if(token->kind != TK_INT) {
      error("関数の引数に型がないよ");
    } else {
      token = token->next;
    }

    argv[i] = expr();
    i++;

    // 最後が,じゃなかったら引数はもうないとみなす
    if(!consume(",")) {
      expect(")");
      break;
    }
  }
}

// node->kind = ND_FUNC
// node->argv = 引数
// node->lhs  = NULL
// node->rhs  = NULL
Node* if_token_is_func_return_node(Token *tok) {
  if(is_supposed_token("(", tok->next)) {
    expect("(");

    Node *node = calloc(1, sizeof(Node));
    parse_argv(node->argv);
    node->kind = ND_FUNC;
    // callocは終端文字文を意識して+1してるけど、意味があるのかしら
    node->funcName = calloc(1, tok->len + 1);
    strncpy(node->funcName, tok->str, tok->len);
    return node;
  }
  return NULL;
}

// node->kind = ND_LVAR
// node->lhs  = NULL
// node->rhs  = NULL
Node* if_token_is_lvar_return_node(Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  // 変数
  node->kind = ND_LVAR;
  Lvar *lvar = create_or_set_lvars(tok);
  node->offset = lvar->offset;
  return node;
}

Node* return_func_or_lvar_node(Token *tok) {
    Node *node;

    node = if_token_is_func_return_node(tok);
    if(node) return node;

    node = if_token_is_lvar_return_node(tok);
    return node;
}

// term = "(" expr ")" | type ident ( "(" ")" )? |ident ( "(" ")" )? | num
Node *term() {
  if(consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  // 宣言
  if(token->kind == TK_INT) {
    token = token->next;
    Token *tok = consume_ident();
    if(!tok) error("型の後に変数がないよ");
    return return_func_or_lvar_node(tok);
  }

  // 実装 
  Token *tok = consume_ident();
  if(tok) {
    return return_func_or_lvar_node(tok);
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

  if(consume("&"))
    return new_node(ND_ADDR, term(), NULL);

  if(consume("*"))
    return new_node(ND_DEREF, term(), NULL);

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
  Node *node = eqaulity();
  if(consume("=")) {
    return new_node(ND_ASSIGN, node, assign());
  }

  return node;
}

Node *expr() { return assign(); }

Node *if_else_statement() {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_IF_ELSE_STMT;
  node->lhs = stmt();
  expect("else");
  node->rhs = stmt();
  return node;
}

Node *if_statement() {

  Node *node = calloc(1, sizeof(Node));
  // ifのNode種別は、elseがあるかどうかで設定値がかわるので、とっておく
  Node *ifNode = node;

  expect("(");
  node->lhs = expr();
  expect(")");

  // elseがあるかを確認するため、if(expr) stmt else stmt
  //                                   ↑を進めとく
  Token *currentToken = token;
  stmt();

  if(is_supposed_token("else", token)) {
    // ifelseの場合のNodeの構成はこんな感じ
    //       if
    //  expr   ifelse
    //       stmt(then) stmt(else)
    //
    ifNode->kind = ND_IF_ELSE;
    // 進めたtokenを戻す
    token = currentToken;
    node->rhs = if_else_statement();
  } else {
    // ifの場合のNodeの構成はこんな感じ
    //       if
    //  expr  stmt
    node->kind = ND_IF;
    // 進めたtokenを戻す
    token = currentToken;
    node->rhs = stmt();
  }
  return node;
}

Node *while_statement() {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_WHILE;
  expect("(");
  node->lhs = expr();
  expect(")");
  node->rhs = stmt();
  return node;
}

Node *for_stmt_statement() {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FOR_STMT;
  node->lhs = stmt();
  return node;
}

Node *for_loop_statement() {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FOR_LOOP;

  // 後処理がない場合
  if(consume(")")) {
    node->lhs = NULL;
  } else {
    node->lhs = expr();
    expect(")");
  }

  node->rhs = for_stmt_statement();
  return node;
}

Node *for_continue_statement() {

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FOR_CONTINUE;

  // 継続条件がない場合
  if(consume(";")) {
    node->lhs = NULL;
  } else {
    node->lhs = expr();
    expect(";");
  }

  node->rhs = for_loop_statement();
  return node;
}

// 現状、以下の構文をサポートしている
// for(expr; expr; expr) stmt()
// for(; expr; expr) stmt()
// for(;; expr) stmt()
// for(;;) stmt()
Node *for_statement() {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FOR;
  expect("(");

  // 初期化式がない場合
  if(consume(";")) {
    node->lhs = NULL;
  } else {
    node->lhs = expr();
    expect(";");
  }

  node->rhs = for_continue_statement();
  return node;
}

Node *block_statement() {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_BLOCK;

  // ブロック構文のステートメントを格納する配列
  // ひとまず固定の長さにする

  // 動的配列で確保できるともっといいね。
  // 動的配列をvectorっていうのかな。

  // ポインタを格納する配列へのポインタ
  Node **stmtArray = calloc(100, sizeof(node));

  int stmtIndex = 0;
  while(!consume("}")) {
    stmtArray[stmtIndex] = stmt();
    stmtIndex++;
    if(stmtIndex > 100) {
      error("ブロック構文内のstmtは100個まで");
    }
  }
  node->vector = stmtArray;
  return node;
}

Node *expect_func_difinition() {

  if(token->kind != TK_INT) 
    error("トップレベルの関数に型がないよ");
  token = token->next;

  Token *tok = consume_ident();

  if(!tok)
    error("トップレベルは関数しか書けないよ");

  expect("(");

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FUNC_DIF;
  node->funcName = calloc(1, tok->len + 1);
  strncpy(node->funcName, tok->str, tok->len);

  // 引数をパース )も読み飛ばしてる
  parse_argv_with_type(node->argv);

  expect("{");

  return node;
}

// 現在パースしているトークンがトップレベルがどうかを判断する
bool isTopLebel = true;

// statementは以下のいずれかを想定している
// return;
// if(expr) statement;
// while(expr) statement;
// for(expr; expr; expr;) statement;
// expr;
Node *stmt() {
  Node *node;

  if(isTopLebel) {
    node = expect_func_difinition();
    isTopLebel = false;
    return node;
  }

  if(consume("return")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
  } else if(consume("if")) {
    node = if_statement();
    return node;
  } else if(consume("while")) {
    node = while_statement();
    return node;
  } else if(consume("for")) {
    node = for_statement();
    return node;
    // ブロック構文
  } else if(consume("{")) {
    node = block_statement();
    return node;
    // 関数の終了を表すブロック構文
  } else if(consume("}")) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_FUNC_DIF_END;
    isTopLebel = true;
    // 関数ごとに変数名を管理するために、functionIDを降る
    functionId++;
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
