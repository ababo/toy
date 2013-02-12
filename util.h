#ifndef UTIL_H
#define UTIL_H

typedef __builtin_va_list va_list;
#define va_start(vargs, last_param) __builtin_va_start(vargs, last_param)
#define va_end(vargs) __builtin_va_end(vargs)
#define va_arg(vargs, arg_type) __builtin_va_arg(vargs, arg_type)

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

#define asm __asm__ __volatile__

static inline void outb(uint16_t port, uint8_t value) {
  asm("outb %%al, %%dx" : : "a"(value), "d"(port));
}

static inline volatile uint64_t rdmsr(uint32_t msr) {
  uint32_t low, high;
  asm("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
  return ((uint64_t)high << 32) + low;
}

static inline void wrmsr(uint32_t msr, uint64_t value) {
  uint32_t low = (uint32_t)value, high = (uint32_t)(value >> 32);
  asm("wrmsr" : : "a"(low), "d"(high), "c"(msr));
}

size_t strlen(const char *str);
char *strrev(char *str);

void *memset(void *ptr, int value, size_t num);
void *memcpy(void *dst, const void *src, size_t num);

// negative radix means uppercase result string
char *ultoa(unsigned long value, char *buf, int radix);

#define ROUND_DIV(dividend, divisor) \
  (((dividend) + (divisor) - 1) / (divisor))
#define BLOCK_NUM(num, num_per_block) \
  (((num) + (num_per_block) - 1) / (num_per_block))
#define INT_BITS(value, low, high)              \
  (value << (sizeof(value) * 8 - 1 - high) >>   \
   (sizeof(value) * 8 - 1 - high + low))

#endif // UTIL_H
