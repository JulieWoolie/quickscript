#include <stdio.h>

typedef unsigned int uint32;
typedef int int32;

static uint32 fibonacci(const uint32 n) {
  if (n == 0) return 0;
  if (n == 1) return 1;

  uint32 gparent = 0;
  uint32 parent = 1;

  for (uint32 i = 0; i < n - 1; i++) {
    const uint32 r = gparent + parent;
    gparent = parent;
    parent = r;
  }

  return parent;
}

int32 main() {
  printf("Result: %d\n", fibonacci(15));
  return 0;
}