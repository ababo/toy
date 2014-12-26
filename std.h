/*
 * Kernel's subset of stdlib.
 */

#ifndef __STD_H_
#define __STD_H_

/* stdarg.h */

typedef __builtin_va_list va_list;

#define va_start(vargs, last_param) __builtin_va_start(vargs, last_param)
#define va_arg(vargs, arg_type) __builtin_va_arg(vargs, arg_type)
#define va_end(vargs) __builtin_va_end(vargs)

/* stdbool.h */

typedef char bool;
static const bool false = 0;
static const bool true = 1;

/* stddef.h */

typedef unsigned long size_t;
typedef long ptrdiff_t;

/* stdint.h */

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#ifdef __LP64__
typedef long int64_t;
typedef unsigned long uint64_t;
#else
typedef long long int64_t;
typedef unsigned long long uint64_t;
#endif // __LP64__

/* stdlib.h */

#define NULL 0

char *ultoa(unsigned long value, char *buf, int radix);

/* string.h */

void *memset(void *ptr, int value, size_t size);
size_t strlen(const char *str);

#endif /* __STD_H_ */
