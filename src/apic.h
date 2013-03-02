#ifndef APIC_H
#define APIC_H

#include "util.h"

int get_apic_id(void);
bool is_bootstrap_cpu(void);

void set_apic_eoi(void);

// interval is given in microseconds
void start_apic_timer(int interval, bool periodic);
void stop_apic_timer(void);

bool start_ap_cpu(int apic_id, int startup_page_addr);

void init_apic(void);

#endif // APIC_H
