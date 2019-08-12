#include "stack.h"

int stack[10];
int rsp = 0;

int push(int value) {
  stack[rsp] = value;
  rsp++;
  return value;
}

int pop() {
  rsp--;
  return stack[rsp];
}