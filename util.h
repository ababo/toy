#ifndef UTIL_H
#define UTIL_H

typedef int bool;

#define true 1
#define false 0

typedef unsigned long size_t;

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long int64_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;

static inline void outb(unsigned short port, unsigned char data) {
  __asm__("outb %%al, %%dx" : : "a"(data), "d"(port));
}

size_t strlen(const char *str);
char *strrev(char *str);
char *ultoa(unsigned long value, char *buf, int radix);

void *memset(void *ptr, int value, size_t num);

typedef __builtin_va_list va_list;
#define va_start(vargs, last_param) __builtin_va_start(vargs, last_param)
#define va_end(vargs) __builtin_va_end(vargs)
#define va_arg(vargs, arg_type) __builtin_va_arg(vargs, arg_type)

#endif // UTIL_H
