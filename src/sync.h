#ifndef SYNC_H
#define SYNC_H

#include "util.h"

struct spinlock {
  volatile int busy;
};

static inline void create_spinlock(struct spinlock *lock) {
  lock->busy = false;
}

static inline void acquire_spinlock(struct spinlock *lock) {
  ASMV("1: movl $1, %%eax\nxchgl %%eax, %0\ntest %%eax, %%eax\n"
       "jz 2f\npause\njmp 1b\n2:" : "+m"(lock->busy) : : "eax");
}

static inline void release_spinlock(struct spinlock *lock) {
  ASMV("xorl %%eax, %%eax\nxchgl %%eax, %0" : "+m"(lock->busy) : : "eax");
}

#endif // SYNC_H
