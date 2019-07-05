#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの型を表す値
typedef enum {
  TK_RESERVED,  // 記号
  TK_NUM,       // 整数トークン
  TK_EOF,       // 入力の終わりを表すトークン
} TokenKind;

// 抽象構文木のノードの種類
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_LT,  // <
  ND_LTE, // <=
  ND_GT,  // >
  ND_GTE, // >=
  ND_NUM, // 整数
} NodeKind;

// Node型の中にNodeがある
// typef struct Node　としておくと、lhsとかでNode型がわからないことによるwarningが消えた
typedef struct Node
{
  NodeKind kind;    // 演算子かND_NUM
  struct Node *lhs; // 左辺
  struct Node *rhs; // 右辺
  int val;          // kindがND_NUMの場合のみ使う
} Node;


// トークンの型
typedef struct Token {
  TokenKind kind;        // トークンの型
  struct Token *next;    // 次の入力トークン 
  int val;               // kindがTK_NUMの場合、その数値
  char *str;             // トークン文字列 (エラーメッセージ用)
  int len;               // トークンの長さ kindがTK_RESERVEDのときのみ桁数をセット
} Token;

// 現在着目しているトークン
Token *token;

// 入力プログラム
char *user_input;

// 現在のtokenのindex
int pos = 0;

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
  // pos分スベースを出力する
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

Node *expr();

Node *term() {
  if(consume("(")) {
    Node *node = expr();
    expect(")");
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
      node = new_node(ND_GT, node, add());
    else if(consume(">="))
      node = new_node(ND_GTE, node, add());
    else
      return node;
  }
}


Node *expr() {
  return relational();
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

    if(*p ==  '>' && !memcmp(p, ">=", 2)) {
      cur = new_token(TK_RESERVED, cur, p);
      cur->len = 2;
      p += 2;
      continue;
    }

    if(*p ==  '<' && !memcmp(p, "<=", 2)) {
      cur = new_token(TK_RESERVED, cur, p);
      cur->len = 2;
      p += 2;
      continue;
    }

    if(*p == '+' || *p == '-'  || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
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

    error_at(p, "トークナイスできません");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

// 以下のノードを考えてみよう
//       -
//    +    4
//  +   3 
// 1 2
//
// 最初は、演算子なので、gen(node->lhs) gen(node->rhs)が走る
// 1.スタックには、gen(node->rhs)で右辺の数値の4がpushされる
// 2.左辺は、また演算子なので、gen(node->lhs) gen(node->rhs)が走る
// 3.スタックには、右辺の数値3がpushされる
// 4.左辺はまた演算子なので、gen(node-lhs) gen(node->rhs)が走る
// 5.スタックには、左辺の数値1、右辺の数値2がpushされる
// このときのスタックは以下の通り
//  
// 1
// 2 
// 3
// 4
//
// pop rdi  ※1を取り出す
// pop rdx  ※2を取り出す
// 演算子の型を見て、+ なら、rdiとraxを足して、raxに格納する
// 演算子の型を見て、- なら、略...
// 結果のraxを再度スタックに格納
// 4の再帰が、同様にpopして、結果をスタックに格納
// 2の再帰が、略...

void gen(Node *node) {
  if(node->kind == ND_NUM) {
    // 数値だったらスタックにpush
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      //  1 + 2 / 2の場合、構文木はこう
      //
      //    +
      //  1   /
      //     2  3
      //
      // スタックは、以下の順に積まれる
      //  3
      //  2
      //  1
      //
      // idiv rax rdi という命令セットはないとのこと
      // idivはrdx とRDXとRAXを連結して128ビット整数にする
      // それを引数のrdiレジスタの64ビットの値で割り、商をRAXにあまりをRDXにセットする
      // cqoを使うと、RAXを128ビットに伸ばして、RDXとRAXにセットすることができる
      // まとめると、 2 / 3の場合
      // pop rdi ⇨ 3
      // pop rax ⇨ 2
      // raxの2を128ビットに拡張して、 2の値を、 rdiの3でわる。結果がraxにセットされる

      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
    case ND_LTE:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
  }

  printf("  push rax\n");
}

int main(int argc, char **argv) {
  if(argc != 2) {
    fprintf(stderr, "引数の数が正しくありません\n");
    return 1;
  }
  // トークナイズする
  token = tokenize(argv[1]);
  Node *node = expr();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  gen(node);

  // スタックトップに式全体の値が残っているはずなので
  // それをRAXにロードして関数からの返り値とする
  printf("  pop rax\n");
  printf("  ret\n");

  return 0;
}

