#ifndef SYNC_H
#define SYNC_H

#include "util.h"
#include "schedule.h"

struct spinlock {
  volatile uint8_t busy;
};

static inline void create_spinlock(struct spinlock *lock) {
  lock->busy = false;
}

// zero tries means (practically) forever
static inline bool acquire_spinlock(struct spinlock *lock, uint64_t tries) {
  uint8_t al;
  ASMV("movq %2, %%rcx\ncli\n1: movb $1, %%al\nxchgb %%al, %0\n"
       "testb %%al, %%al\njz 2f\ndecq %%rcx\njz 2f\npause\njmp 1b\n"
       "2: testb %%al, %%al\njnz 3f\nsti\n3: movb %%al, %1"
       : "+m"(lock->busy), "=m"(al) : "m"(tries) : "rcx", "al");
  return !al;
}

static inline void release_spinlock(struct spinlock *lock) {
  ASMV("xorb %%al, %%al\nxchgb %%al, %0\nsti" : "+m"(lock->busy) : : "al");
}

#endif // SYNC_H
