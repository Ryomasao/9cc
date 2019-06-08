#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if(argc != 2) {
    fprintf(stderr, "引数の数が正しくありません\n");
    return 1;
  }

  char *p = argv[1];

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");


  // pは文字列の先頭アドレスを格納している変数
  // もう一回書く。アドレスを格納している変数だ
  // &p はその変数のアドレス
  // strtolは文字列のアドレスを受け取って、10進数の値を先頭から探す
  // 10進数以外の値が見つかったら、その文字列のアドレスをpに格納して返す
  // アドレスを書き換えたいから、pそのもののアドレスを渡すんだね
  printf(" mov rax, %ld\n", strtol(p, &p, 10));

  // pは何を指しているかdebug
  //printf("remain %s\n", p);

  while (*p) {
    if(*p == '+') {
      p++;
      printf(" add rax, %ld\n", strtol(p, &p, 10));
      continue;
    }

    if(*p == '-') {
      p++;
      printf(" sub rax, %ld\n", strtol(p, &p, 10));
      continue;
    }

    fprintf(stderr, "予期しない文字です: '%c'\n", *p);
    return -1;
  }

  printf(" ret \n");
  return 0;
}
