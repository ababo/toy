#include "std.h"

static char *reverse(char *str) {
  char tmp;
  size_t len = strlen(str), half = len / 2;
  for (size_t i = 0, j = len - 1; i < half; i++, j = len - i - 1) {
    tmp = str[i];
    str[i] = str[j];
    str[j] = tmp;
  }
  return str;
}

char *ultoa(unsigned long value, char *buf, int radix) {
  char* str = buf, lbase;
  if (radix > 0) lbase = 'a';
  else lbase = 'A', radix = -radix;

  if (radix < 2 || radix > 36) {
    *buf = 0;
    return buf;
  }

  lbase -= 10;
  do {
    int rem = value % radix;
    *buf++ = rem + (rem < 10 ? '0' : lbase);
  }
  while (value /= radix);

  *buf = 0;
  return reverse(str);
}

void *memset(void *ptr, int value, size_t size) {
  volatile char *p = (char*)ptr, v = (char)value;
  while (size-- > 0) *p++ = v;
  return ptr;
}

size_t strlen(const char *str) {
  const char *chr = str;
  while (*chr) chr++;
  return (size_t)(chr - str);
}
