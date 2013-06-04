#include "cpu.h"

static inline int get_cpu(void) {
  uint32_t ebx;
  extern uint8_t __cpu_indexes[];
  ASMV("movl $1, %%eax; cpuid" : "=b"(ebx) : : "eax", "ecx", "edx");
  return __cpu_indexes[INT_BITS(ebx, 24, 31)];
}

static inline int get_bsp_cpu(void) {
  extern int __bsp_cpu;
  return __bsp_cpu;
}
