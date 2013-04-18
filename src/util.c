#include "util.h"

char *strcat(char *dst, const char *src) {
  strcpy(dst + strlen(dst), src);
  return dst;
}

char *strcpy(char *dst, const char *src) {
  char *tmp;
  for (tmp = dst; *src; *tmp++ = *src++);
  *tmp = 0;
  return dst;
}

size_t strlen(const char *str) {
  const char *chr = str;
  while (*chr)
    chr++;
  return (size_t)(chr - str);
}

char *strrev(char *str) {
  char tmp;
  size_t len = strlen(str), half = len / 2;
  for (size_t i = 0, j = len - 1; i < half; i++, j = len - i - 1)
    tmp = str[i], str[i] = str[j], str[j] = tmp;
  return str;
}

void *memset(void *ptr, int value, size_t size) {
  for (; size > 0; size--)
    *(uint8_t*)ptr++ = (uint8_t)value;
  return ptr;
}

void *memcpy(void *dst, const void *src, size_t size) {
  uint8_t *tmp = dst;
  for (; size > 0; size--)
    *tmp++ = *(uint8_t*)src++;
  return dst;
}

int memcmp(const void *buf1, const void *buf2, size_t size) {
  for (size_t i = 0; i < size; i++)
    if (((char*)buf1)[i] > ((char*)buf2)[i])
      return 1;
    else if (((char*)buf1)[i] < ((char*)buf2)[i])
      return -1;
  return 0;
}

void *memmem(const void *buf1, size_t size1, const void *buf2, size_t size2) {
  for(; size1 >= size2; size1--, buf1++)
    if (!memcmp(buf1, buf2, size2))
      return (void*)buf1;
  return NULL;
}

char *ultoa(unsigned long value, char *buf, int radix) {
  char *str = buf, lbase;
  if (radix > 0)
    lbase = 'a';
  else
    lbase = 'A', radix = -radix;

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
  return strrev(str);
}
