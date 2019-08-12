#include <stdio.h>
#include "stack.h"

int main() {
  push(1);
  push(2);
  printf("pop:%d\n", pop());
  printf("pop:%d\n", pop());
  return 1;
}