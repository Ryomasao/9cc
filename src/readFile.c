#include "9cc.h"

int readFile(char *path, char code[][MAX_COLUMN]) {
  FILE *fp;

  fp = fopen(path, "r");

  if(fp == NULL) {
    printf("ファイルのオープンに失敗しました\n");
    return -1;
  }

  int i = 0;
  while ((fgets(code[i], MAX_COLUMN, fp)) != NULL)
  {
    // debug
    //printf("INPUT:%s", code[i]);
    i++;
  }
  code[i][0] = EOF;
  fclose(fp);
  return 1;
}