#include "config.h"
#include "sync.h"

struct spinlock *__outer_spinlocks[CONFIG_CPUS_MAX] = { };
