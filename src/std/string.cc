#include "std/string.h"

extern "C" {

void* memset(void* ptr, int value, size_t size) {
  volatile char* p = static_cast<char*>(ptr);
  char v = static_cast<char>(value);
  while (size-- > 0) *p++ = v;
  return ptr;
}

size_t strlen(const char* str) {
  const char* chr = str;
  while (*chr) chr++;
  return static_cast<size_t>(chr - str);
}

char* _strrev(char* str) {
  char tmp;
  size_t len = strlen(str), half = len / 2;
  for (size_t i = 0, j = len - 1; i < half; i++, j = len - i - 1) {
    tmp = str[i], str[i] = str[j], str[j] = tmp;
  }
  return str;
}

}
