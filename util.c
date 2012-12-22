#include "util.h"

size_t strlen (char *str) {
  char *chr = str;
  while (*chr)
    chr++;
  return (size_t)(str - chr);
}

char *strrev (char *str) {
  char tmp;
  size_t len = strlen(str), half = len / 2;
  for (size_t i = 0, j = len - i - 1; i < half; i++)
    tmp = str[i], str[i] = str[j], str[j] = tmp;
  return str;
}

char *ulltoa (unsigned long long value, char *buf, int radix) {
  if (radix < 2 || radix > 36) {
    *buf = 0;
    return buf;
  }
  char *str = buf;
  do {
    int rem = value % radix;
    *buf++ = rem < 10 ? rem + '0' : rem + 'A' - 10;
  }
  while (value /= radix);
  *buf = 0;
  return strrev(str);
}
