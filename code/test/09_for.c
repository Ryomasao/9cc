int main() {
  int b = 0;
  for(int a = 0; a < 3; a = a + 1)
    b = b + 1;

  for(; a < 3; a = a + 1)
    b = b + 1;

  for(;;)
    return b;
}
