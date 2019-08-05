#include "9cc.h"

int main(int argc, char **argv) {
  if(argc != 2) {
    fprintf(stderr, "引数の数が正しくありません\n");
    return 1;
  }

  // ユーザー入力値をグローバル変数として取っておく
  // エラー関数用
  user_input = argv[1];
  int mode = argv[2];

  char *filePath = argv[1];
  char input[MAX_LINE][MAX_COLUMN] = {};
  
  if(!readFile(filePath, input))
    return -1;

  // ローカル変数格納用の変数の初期設定
  // これにより、localsの先頭はゴミデータになっちゃうので微妙
  locals = calloc(1, sizeof(Lvar));
  locals->next = NULL;

  // トークナイズする
  tokenize(input);
  program();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // プロローグ処理
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  // 変数26個分の領域を確保する
  printf("  sub rsp, 208\n");

  for(int i = 0; code[i]; i++) {
    gen(code[i]);
    // スタックトップに式全体の値が残っているはずなので
    // それをRAXにロードして関数からの返り値とする
    printf("  pop rax\n");
  }


  // エピローグ
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");

  return 1;
}

