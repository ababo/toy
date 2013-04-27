#ifndef SYNC_H
#define SYNC_H

#include "cpu_info.h"
#include "util.h"

struct spinlock {
  volatile uint8_t busy;
};

static inline void create_spinlock(struct spinlock *lock) {
  lock->busy = false;
}

// zero tries means (practically) forever
static inline bool acquire_spinlock_int(struct spinlock *lock,
                                        uint64_t tries) {
  uint8_t al;
  do ASMV("mov $1, %%al\nxchgb %%al, %0" : "+m"(lock->busy), "=&a"(al));
  while (al && --tries);
  return !al;
}

static inline void release_spinlock_int(struct spinlock *lock) {
  ASMV("xorb %%al, %%al\nxchgb %%al, %0" : "+m"(lock->busy) : : "al");
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

struct mutex {
  struct __mutex_node *head, *tail;
  struct spinlock mlock, ilock;
};

static inline void create_mutex(struct mutex *mutex) {
  mutex->head = mutex->tail = NULL;
  create_spinlock(&mutex->mlock);
  create_spinlock(&mutex->ilock);
}

static inline err_code acquire_mutex(struct mutex *mutex) {
  extern err_code __sleep_in_mutex(struct mutex *mutex);
  if (!acquire_spinlock_int(&mutex->mlock, 1000))
    return __sleep_in_mutex(mutex);
  return ERR_NONE;
}

static inline void release_mutex(struct mutex *mutex) {
  extern void __awake_in_mutex(struct mutex *mutex);
  acquire_spinlock(&mutex->ilock, 0);
  if (mutex->tail)
    __awake_in_mutex(mutex);
  else
    release_spinlock_int(&mutex->mlock);
  release_spinlock(&mutex->ilock);
}

err_code sleep(uint64_t period); // in microseconds

void init_sync(void);

#endif // SYNC_H
