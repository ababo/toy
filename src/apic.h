#ifndef APIC_H
#define APIC_H

#include "util.h"

void set_apic_eoi(void);

// interval is given in microseconds
void start_apic_timer(int interval, bool periodic);
void stop_apic_timer(void);

// startup address must be 4096-aligned
bool start_ap_cpu(int apic_id, int startup_addr, volatile int *started_cpus);

void init_apic(void);

#endif // APIC_H
