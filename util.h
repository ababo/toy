#ifndef UTIL_H
#define UTIL_H

typedef int bool;

#define true 1
#define false 0

static inline void outb (unsigned short port, unsigned char data) {
  __asm("outb %%al, %%dx" : : "a"(data), "d"(port));
}

#endif // UTIL_H
