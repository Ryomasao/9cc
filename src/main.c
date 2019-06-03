#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if(argc != 2) {
    fprintf(stderr, "引数の数が正しくありません\n");
    return 1;
  }


  // C言語で、とある言語で書かれたファイルを読み込んで
  // アセンブリ言語に変換するイメージ

  // ここでは単純な整数を引数にとり、それをreturnするだけのもの
  // raxは関数の返り値を格納するレジスタ

  // 直接機械語にコンパイルするかと思ったけれども、アセンブリにするんだね

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");
  printf("  mov rax, %d\n", atoi(argv[1]));
  printf("  ret \n");


  // gcc main main.c
  // ./main 123 > test.s
  // アセンブリを機械語にする。
  // 面白いのは、このアセンブリが123をreturnするプログラムってところ
  // gcc test test.s
  // ./test
  // echo $?

  return 0;
}
