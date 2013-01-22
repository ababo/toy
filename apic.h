#ifndef APIC_H
#define APIC_H

#include "util.h"

void init_apic(void);

typedef void (*timer_callback)(void);

void add_timer(bool periodic, uint64_t period, timer_callback callback);
void remove_timer(timer_callback callback);

#endif // APIC_H
