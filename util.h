#ifndef UTIL_H
#define UTIL_H

typedef int bool;

#define true 1
#define false 0

typedef unsigned long long size_t;

static inline void outb (unsigned short port, unsigned char data) {
  __asm("outb %%al, %%dx" : : "a"(data), "d"(port));
}

size_t strlen (char *str);
char *strrev (char *str);
char *ulltoa (unsigned long long value, char *buf, int radix);

#endif // UTIL_H
