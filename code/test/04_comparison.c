int main() {
  int a = 0 < 1;
  int b = 1 < 1;
  int c = 1 <= 1;
  int d = 1 <= 0;

  int e = 0 > 1;
  int f = 1 > 1;
  int g = 1 >= 1;
  int h = 1 >= 0;

  return a + b + c + d + e + f + g + h;
}