#ifndef SYNC_H
#define SYNC_H

#include "util.h"

struct spinlock {
  volatile uint8_t busy;
};

static inline void create_spinlock(struct spinlock *lock) {
  lock->busy = false;
}

// zero tries means (practically) forever
static inline bool acquire_spinlock(struct spinlock *lock, uint64_t tries) {
  extern struct spinlock *__if_owner;
  if (!__if_owner) {
    __if_owner = lock;
    ASMV("cli");
  }

  uint8_t al;
  do ASMV("mov $1, %%al\nxchgb %%al, %0" : "+m"(lock->busy), "=m"(al));
  while (al && !--tries);

  if (al && __if_owner == lock) {
    __if_owner = NULL;
    ASMV("sti");
  }

  return !al;
}

static inline void release_spinlock(struct spinlock *lock) {
  ASMV("xorb %%al, %%al\nxchgb %%al, %0" : "+m"(lock->busy) : : "al");

  extern struct spinlock *__if_owner;
  if (__if_owner == lock) {
    __if_owner = NULL;
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
  if (!acquire_spinlock(&mutex->mlock, 1000))
    return __sleep_in_mutex(mutex);
  return ERR_NONE;
}

static inline void release_mutex(struct mutex *mutex) {
  extern void __awake_in_mutex(struct mutex *mutex);
  acquire_spinlock(&mutex->ilock, 0);
  if (mutex->tail)
    __awake_in_mutex(mutex);
  else
    release_spinlock(&mutex->mlock);
  release_spinlock(&mutex->ilock);
}

void init_sync(void);

#endif // SYNC_H
