#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct test {
  long a;
  int b;
} t1, t2;

int main (int argc, char const* argv[]) {
  t1.a = (long) 1;
  t1.b = 10;

  t2 = t1;
  printf("before; a = %ld, b = %ld\n", t2.a, t2.b);
  t1.b = 100;
  printf("after; a = %ld, b = %ld\n", t2.a, t2.b);
  return 0;
}
