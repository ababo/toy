#ifndef CPU_INFO_H
#define CPU_INFO_H

#include "util.h"

#define CPU_VENDOR_UNKNOWN -1
#define CPU_VENDOR_INTEL 1
#define CPU_VENDOR_AMD 2

struct cpu_desc {
  uint8_t apic_id;
  uint8_t thread;
  uint8_t core;
  uint8_t chip;
  uint32_t domain;
};

int get_cpu_vendor(void);
int get_cpus(void);
int get_cpu_bsp(void);
const struct cpu_desc *get_cpu_info(void);

void init_cpu_info(void);

#endif // CPU_INFO_H
