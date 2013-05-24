#include "config.h"
#include "sync.h"

INTERNAL struct spinlock *__outer_spinlocks[CONFIG_CPUS_MAX] = { };
