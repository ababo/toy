#ifndef __CPU_INFO_H
#define __CPU_INFO_H

#include "common.h"
#include ARCH_FILE(/cpu_info.inc)

#define CPU_VENDOR_UNKNOWN 0
#define CPU_VENDOR_INTEL 1
#define CPU_VENDOR_AMD 2

struct cpu_desc {
  BY_ARCH uint8_t id;
  uint8_t thread;
  uint8_t core;
  uint8_t chip;
  uint32_t domain;
};

static inline int get_cpu(void);
static inline int get_bsp_cpu(void);

int get_cpu_vendor(void);

int get_cpus(void);
const struct cpu_desc *get_cpu_desc(int cpu);

void init_cpu_info(void);

#endif // __CPU_INFO_H
