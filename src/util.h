#ifndef UTIL_H
#define UTIL_H

#define IN
#define OUT
#define IN_OUT
#define INTERNAL

#define STR(x) #x
#define STR_EXPAND(x) STR(x)

#define ALIGNED(n) __attribute__((aligned(n)))
#define NOINLINE __attribute__((noinline))
#define PACKED __attribute__((packed))
#define SECTION(name) __attribute__((section(name)))
#define UNUSED __attribute__((unused))

typedef __builtin_va_list va_list;
#define va_start(vargs, last_param) __builtin_va_start(vargs, last_param)
#define va_end(vargs) __builtin_va_end(vargs)
#define va_arg(vargs, arg_type) __builtin_va_arg(vargs, arg_type)

#define NULL (void*)0

typedef _Bool bool;
#define true (bool)1
#define false (bool)0

static inline const char *bool_str(bool value) {
  return value ? "true" : "false";
}

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#ifdef __LP64__
typedef long int64_t;
typedef unsigned long uint64_t;
typedef unsigned long size_t;
#else
typedef long long int64_t;
typedef unsigned long long uint64_t;
typedef unsigned int size_t;
#endif

#define INT8_MAX 0x7F
#define INT8_MIN (-INT8_MAX - 1)
#define INT16_MAX 0x7FFF
#define INT16_MIN (-INT16_MAX - 1)
#define INT32_MAX 0x7FFFFFFF
#define INT32_MIN (-INT32_MAX - 1)
#define INT64_MAX 0x7FFFFFFFFFFFFFFFL
#define INT64_MIN (-INT64_MAX - 1L)
#define UINT8_MAX (uint8_t)0xFF
#define UINT16_MAX (uint16_t)0xFFFF
#define UINT32_MAX (uint32_t)0xFFFFFFFF
#define UINT64_MAX (uint64_t)0xFFFFFFFFFFFFFFFFL

#define ABS(num) ((num) < 0 ? (-num) : (num))

#define INT_BITS(value, low, high)              \
  (value << (sizeof(value) * 8 - 1 - high) >>   \
   (sizeof(value) * 8 - 1 - high + low))

#define ROUND_DIV(dividend, divisor) \
  (((dividend) + (divisor) - 1) / (divisor))

// minimal number of elements with total size >= given size
#define SIZE_ELEMENTS(size, element_size) \
  (((size) + (element_size) - 1) / (element_size))

#define BIT_ARRAY_SET(array, bit)                       \
  ((uint8_t*)array)[(bit) / 8] |= 1 << ((bit) % 8);
#define BIT_ARRAY_RESET(array, bit)                     \
  ((uint8_t*)array)[(bit) / 8] &= ~(1 << ((bit) % 8));
#define BIT_ARRAY_GET(array, bit)                       \
  !!(((uint8_t*)array)[(bit) / 8] & (1 << ((bit) % 8)))

#define ASM __asm__
#define ASMV __asm__ __volatile__

static inline int bsrq(uint64_t value) {
  uint64_t index;
  ASMV("bsrq %1, %0" : "=r"(index) : "mr"(value));
  return (int)index;
}

static inline int bsfq(uint64_t value) {
  uint64_t index;
  ASMV("bsfq %1, %0" : "=r"(index) : "mr"(value));
  return (int)index;
}

static inline uint8_t inb(uint16_t port) {
  uint8_t value;
  ASMV("inb %%dx, %%al" : "=a"(value) : "d"(port));
  return value;
}

static inline uint16_t inw(uint16_t port) {
  uint16_t value;
  ASMV("inw %%dx, %%ax" : "=a"(value) : "d"(port));
  return value;
}

static inline void outb(uint16_t port, uint8_t value) {
  ASMV("outb %%al, %%dx" : : "a"(value), "d"(port));
}

static inline void outw(uint16_t port, uint16_t value) {
  ASMV("outw %%ax, %%dx" : : "a"(value), "d"(port));
}

static inline uint64_t rdmsr(uint32_t msr) {
  uint32_t low, high;
  ASMV("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
  return ((uint64_t)high << 32) + low;
}

static inline void wrmsr(uint32_t msr, uint64_t value) {
  uint32_t low = (uint32_t)value, high = (uint32_t)(value >> 32);
  ASMV("wrmsr" : : "a"(low), "d"(high), "c"(msr));
}

static inline uint64_t cmpxchgq(uint64_t *ptr, uint64_t old, uint64_t new) {
  uint64_t ret;
  ASMV("lock\ncmpxchgq %1, %2"
       : "=a"(ret) : "r"(new), "m"(*ptr), "0"(old) : "memory");
  return ret;
}

char *strcat(char *dst, const char *src);
char *strcpy(char *dst, const char *src);
size_t strlen(const char *str);
char *strrev(char *str);

void *memset(void *ptr, int value, size_t size);
void *memcpy(void *dst, const void *src, size_t size);
int memcmp(const void *buf1, const void *buf2, size_t size);
void *memmem(const void *buf1, size_t size1, const void *buf2, size_t size2);

// negative radix means uppercase result string
char *ultoa(unsigned long value, char *buf, int radix);

int kputchar(int chr);
int kprintf(const char *format, ...);

#define LOG(severity, format, ...) {     \
    char __buf[256];                     \
    strcpy(__buf, __func__);             \
    strcat(__buf, ": " #severity ": ");  \
    strcat(__buf, format);               \
    strcat(__buf, "\n");                 \
    kprintf(__buf, ##__VA_ARGS__);       \
  }

#ifdef DEBUG
#define LOG_DEBUG(format, ...) LOG(debug, format, ##__VA_ARGS__)
#else
#define LOG_DEBUG(format, ...)
#endif
#define LOG_INFO(format, ...) LOG(info, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) LOG(error, format, ##__VA_ARGS__)

typedef int err_code;
#define ERR_NONE 0
#define ERR_BUSY 1
#define ERR_BAD_INPUT 2
#define ERR_BAD_STATE 3
#define ERR_NOT_FOUND 4
#define ERR_OUT_OF_MEMORY 5

#endif // UTIL_H
