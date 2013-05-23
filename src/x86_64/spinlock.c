#include "../config.h"
#include "spinlock.h"

INTERNAL struct spinlock *__outer_spinlocks[CONFIG_CPUS_MAX] = { };
