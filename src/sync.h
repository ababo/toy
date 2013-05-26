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

#endif // SYNC_H
