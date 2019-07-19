#include "9cc.h"

int main(int argc, char **argv) {
  if(argc != 2) {
    fprintf(stderr, "引数の数が正しくありません\n");
    return 1;
  }

  // ユーザー入力値をグローバル変数として取っておく
  // エラー関数用
  user_input = argv[1];
  // トークナイズする
  token = tokenize(user_input);
  Node *node = expr();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  //gen(node);

  // スタックトップに式全体の値が残っているはずなので
  // それをRAXにロードして関数からの返り値とする
  printf("  pop rax\n");
  printf("  ret\n");

  return 0;
}

