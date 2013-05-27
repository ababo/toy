#ifndef SYNC_H
#define SYNC_H

#include "common.h"
#include ARCH_FILE(/sync.inc)

struct spinlock;

void create_spinlock(struct spinlock *lock);

// does not disable interrupts; zero tries means (practically) forever;
bool acquire_spinlock_int(struct spinlock *lock, uint64_t tries);
void release_spinlock_int(struct spinlock *lock);

// disables interrupts; zero tries means (practically) forever;
bool acquire_spinlock(struct spinlock *lock, uint64_t tries);
void release_spinlock(struct spinlock *lock);

void set_outer_spinlock(bool exists);

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
