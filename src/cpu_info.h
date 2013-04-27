#ifndef CPU_INFO_H
#define CPU_INFO_H

#include "util.h"

#define CPU_VENDOR_UNKNOWN 0
#define CPU_VENDOR_INTEL 1
#define CPU_VENDOR_AMD 2

struct cpu_desc {
  uint8_t apic_id;
  uint8_t thread;
  uint8_t core;
  uint8_t chip;
  uint32_t domain;
};

static inline int get_cpu(void) {
  uint32_t ebx;
  extern uint8_t __cpu_indexes[];
  ASMV("movl $1, %%eax\ncpuid" : "=b"(ebx) : : "eax", "ecx", "edx");
  return __cpu_indexes[INT_BITS(ebx, 24, 31)];
}

static inline int get_bsp_cpu(void) {
  extern int __bsp_cpu;
  return __bsp_cpu;
}

int get_cpu_vendor(void);

int get_cpus(void);
const struct cpu_desc *get_cpu_desc(int cpu);

void init_cpu_info(void);

#endif // CPU_INFO_H
