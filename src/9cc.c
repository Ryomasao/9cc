#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの型を表す値
enum {
  TK_NUM = 256, // 整数トークン
  TK_EOF,       // 入力の終わりを表すトークン
};

enum {
  ND_NUM = 256, // 整数のノードの型
};

// Node型の中にNodeがある
// typef struct Node　としておくと、lhsとかでNode型がわからないことによるwarningが消えた
typedef struct Node
{
  int ty;           // 演算子かND_NUM
  struct Node *lhs; // 左辺
  struct Node *rhs; // 右辺
  int val;          // tyがND_NUMの場合のみ使う
} Node;


// トークンの型
typedef struct {
  int ty;       // トークンの型
  int val;      // tyがTK_NUMの場合、その数値
  char *input;  // トークン文字列 (エラーメッセージ用)
} Token;

// 入力プログラム
char *user_input;

// 現在のtokenのindex
int pos = 0;

// トークナイズした結果のトークン列を格納する配列
// とりあえず100個
Token tokens[100];

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

Node *new_node(int ty, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

int consume(int ty) {
  if(tokens[pos].ty != ty)
    return 0;

  pos++;
  return 1;
}

Node *expr();

Node *term() {
  if(consume('(')) {
    Node *node = expr();
    if(!consume(')')) 
      error_at(tokens[pos].input, "対応する括弧がありません");

    return node;
  }

  if(tokens[pos].ty == TK_NUM) 
    return new_node_num(tokens[pos++].val);

  error_at(tokens[pos].input, "数値じゃないトークンです");
}


Node *mul() {
  Node *node = term();
  for(;;) {
    if(consume('*'))
      node = new_node('*', node, term());
    else if(consume('/'))
      node = new_node('/', node, term());
    else
      return node;
  }
}

Node *expr() {
  Node *node = mul();
  for(;;) {
    if(consume('+'))
      node = new_node('+', node, mul());
    else if(consume('-'))
      node = new_node('-', node, mul());
    else
      return node;
  }
}

// user_inputが指している文字列をトークンに分割して、tokensに保存する
void tokenize() {
  char *p = user_input;
  int i = 0;

  while(*p) {
    // 空白文字はskip
    if(isspace(*p)) {
      p++;
      continue;
    }

    if(*p == '+' || *p == '-'  || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
      tokens[i].ty = *p;
      tokens[i].input = p;
      i++;
      p++;
      continue;
    }

    if(isdigit(*p)) {
      tokens[i].ty = TK_NUM;
      tokens[i].input = p;
      tokens[i].val = strtol(p, &p, 10);
      i++;
      continue;
    }

    error_at(p, "トークナイスできません");
  }

  tokens[i].ty = TK_EOF;
  tokens[i].input = p;
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
  if(node->ty == ND_NUM) {
    // 数値だったらスタックにpush
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->ty) {
    case '+':
      printf("  add rax, rdi\n");
      break;
    case '-':
      printf("  sub rax, rdi\n");
      break;
    case '*':
      printf("  imul rax, rdi\n");
      break;
    case '/':
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
  }

  printf("  push rax\n");
}


int main(int argc, char **argv) {
  if(argc != 2) {
    fprintf(stderr, "引数の数が正しくありません\n");
    return 1;
  }

  // トークナイズする
  user_input = argv[1];
  tokenize();
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
