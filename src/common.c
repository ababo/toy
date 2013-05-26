#include "common.h"
#include "sync.h"

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

void __get_display_size(int *rows, int *cols);
struct spinlock *__get_display_lock(void);

void __set_display_char(int row, int col, char chr);
void __set_display_cursor(int row, int col);

void __shift_display_rows(void);
void __clear_display(void);

static int display_row, display_col;

static void put_char(char chr) {
  int rows, cols;
  __get_display_size(&rows, &cols);

  switch (chr) {
  case '\r':
    display_col = 0;
    break;
  case '\n':
  new_line:
    display_col = 0, display_row++;
    if (display_row == rows) {
      __shift_display_rows();
      display_row--;
    }
    break;
    // TODO: implement other escapes
  default:
    __set_display_char(display_row, display_col, chr);
    if (++display_col == cols)
      goto new_line;
    break;
  }
}

int kprintf(const char *format, ...) {
  acquire_spinlock(__get_display_lock(), 0);

  va_list vargs;
  va_start(vargs, format);

  int num = 0;
  char chr, *str, buf[20];
  long lint;

  struct attributes {
    uint32_t size_long : 1;
  } attrs;

  while ((chr = *format++))
    if (chr == '%') {
      memset(&attrs, 0, sizeof(attrs));

    next_attr_type:
      switch ((chr = *format++)) {
      case 'l':
        attrs.size_long = true;
        goto next_attr_type;

      case '%':
        put_char('%');
        num++;
        break;

      case 's':
        str = va_arg(vargs, char*);
      puts:
        while (*str) {
          put_char(*str++);
          num++;
        }
        break;

      case 'd':
        lint = va_arg(vargs, long);
        if (!attrs.size_long)
          lint = (int)lint;
        if (lint < 0) {
          put_char('-');
          lint = -lint;
          num++;
        }
        str = ultoa(lint, buf, 10);
        goto puts;

      case 'x':
      case 'X':
        lint = va_arg(vargs, long);
        if (!attrs.size_long)
          lint = (unsigned int)lint;
        str = ultoa(lint, buf, chr == 'x' ? 16 : -16);
        goto puts;

      case 'c':
        buf[0] = (char)va_arg(vargs, int), buf[1] = 0, str = buf;
        goto puts;

      // TODO: implement other types and attributes
      }
    }
    else {
      put_char(chr);
      num++;
    }
  va_end(vargs);

  __set_display_cursor(display_row, display_col);

  release_spinlock(__get_display_lock());
  return num;
}

void kclear(void) {
  acquire_spinlock(__get_display_lock(), 0);
  __clear_display();
  __set_display_cursor(0, 0);
  display_row = display_col = 0;
  release_spinlock(__get_display_lock());
}
