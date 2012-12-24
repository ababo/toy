#ifndef UTIL_H
#define UTIL_H

typedef int bool;

#define true 1
#define false 0

typedef unsigned long size_t;

static inline void outb (unsigned short port, unsigned char data) {
  __asm("outb %%al, %%dx" : : "a"(data), "d"(port));
}

size_t strlen (char *str);
char *strrev (char *str);
char *ultoa (unsigned long value, char *buf, int radix);

typedef __builtin_va_list va_list;
#define va_start(vargs, last_param) __builtin_va_start(vargs, last_param)
#define va_end(vargs) __builtin_va_end(vargs)
#define va_arg(vargs, arg_type) __builtin_va_arg(vargs, arg_type)

#endif // UTIL_H
