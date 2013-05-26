#ifndef X86_64_APIC_H
#define X86_64_APIC_H

#include "../common.h"

static inline void set_apic_eoi(void) {
  *(volatile uint32_t*)0xFEE000B0 = 0;
}

void issue_cpu_interrupt(int cpu, int vector);

// interval is given in microseconds
void start_apic_timer(int interval, bool periodic);
void stop_apic_timer(void);

// startup address must be 4096-aligned
bool start_ap_cpu(int cpu, int startup_addr, volatile int *started_cpus);

void init_apic(void);

#endif // X86_64_APIC_H
