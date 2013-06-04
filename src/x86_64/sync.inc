#include "../cpu_info.h"
#include "cpu.h"

struct spinlock {
  volatile uint8_t busy;
};

static inline void create_spinlock(struct spinlock *lock) {
  lock->busy = false;
}

static inline bool acquire_spinlock_int(struct spinlock *lock,
                                        uint64_t tries) {
  uint8_t al;
  do ASMV("mov $1, %%al; xchgb %%al, %0" : "+m"(lock->busy), "=&a"(al));
  while (al && --tries);
  return !al;
}

static inline void release_spinlock_int(struct spinlock *lock) {
  ASMV("xorb %%al, %%al; xchgb %%al, %0" : "+m"(lock->busy) : : "al");
}

static inline bool acquire_spinlock(struct spinlock *lock, uint64_t tries) {
  extern struct spinlock *__outer_spinlocks[];
  struct spinlock **outer = &__outer_spinlocks[get_cpu()];
  if (!cmpxchgq((uint64_t*)outer, 0, (uint64_t)lock))
    ASMV("cli");

  bool acquired = acquire_spinlock_int(lock, tries);

  if (!acquired && *outer == lock) {
    *outer = NULL;
    ASMV("sti");
  }

  return acquired;
}

static inline void release_spinlock(struct spinlock *lock) {
  release_spinlock_int(lock);

  extern struct spinlock *__outer_spinlocks[];
  struct spinlock **outer = &__outer_spinlocks[get_cpu()];
  if (*outer == lock) {
    *outer = NULL;
    ASMV("sti");
  }
}

static inline void set_outer_spinlock(bool exists) {
  extern struct spinlock *__outer_spinlocks[];
  __outer_spinlocks[get_cpu()] = exists ? (struct spinlock*)1 : NULL;
}
