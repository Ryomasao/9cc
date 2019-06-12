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
    return -1;
  pos++;
  return 0;
}

Node *term() {
  if(tokens[pos].ty == TK_NUM) 
    return new_node_num(tokens[pos++].val);
  
  error_at(tokens[pos].input, "数値でも()でもないトークンです");
}

Node *mul() {
  Node *node = term();
  return node;
}

Node *expr() {
  Node *node = term();
  for(;;) {
    if(consume('+'))
      node = new_node('+', node, term());
    else if(consume('-'))
      node = new_node('-', node, term());
    else
      return node;
  }
}

void gen(Node *node) {
  if(node->ty == ND_NUM) {
    printf(" push %d\n", node->val);
    return;
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

    if(*p == '+' || *p == '-') {
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

  // 式の最初は数であることのチェック
  if(tokens[0].ty != TK_NUM)
    error_at(tokens[0].input, "式の最初が数ではありません" );
  
  printf(" mov rax, %d\n", tokens[0].val);

  // 1tokenごとに走査ではなく、 + or -　の後に数というセットでチェックしてる
  // なんとなくtokenごとなのかとと思ったけど、アセンブラに出力するときはadd or sub + 数値だから
  // このセットになる
  // 後から見てわすれるかもしれないけど、tokenの数値は、1文字じゃなくって、次のtokenの区切りまではいってるから注意ね。
  // ex)123+45　
  // 123 token[0]
  // +   token[1]
  // 45  token[2]
  int i = 1;
  while(tokens[i].ty != TK_EOF) {
    if(tokens[i].ty == '+') {
      i++;
      if(tokens[i].ty != TK_NUM)
        error_at(tokens[i].input, "数ではありません");
      printf(" add rax, %d\n", tokens[i].val);
      i++;
      continue;
    }

    if(tokens[i].ty == '-') {
      i++;
      if(tokens[i].ty != TK_NUM)
        error_at(tokens[i].input, "数ではありません");
      printf(" sub rax, %d\n", tokens[i].val);
      i++;
      continue;
    }

    error_at(tokens[i].input, "予期しないトークンです");
  }

  printf(" ret\n");
  return 0;
}
