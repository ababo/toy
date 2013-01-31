#include "apic.h"
#include "display.h"
#include "mem_map.h"
#include "interrupt.h"
#include "util.h"

#define APIC_BASE_MSR 0x1B

#define APIC_SPURIOUS_REG 0x0F0
#define APIC_TIMER_REG 0x320
#define APIC_TIMER_INITIAL_REG 0x380
#define APIC_TIMER_CURRENT_REG 0x390
#define APIC_TIMER_DIVIDE_REG 0x3E0

#define APIC_ENABLE (1 << 11)
#define APIC_LOCAL_ENABLE (1 << 8)
#define APIC_TIMER_MASKED (1 << 16)
#define APIC_TIMER_PERIODIC (1 << 17)

#define APIC_TIMER_DIVIDE_BY_1 0b1011
#define APIC_TIMER_DIVIDE_BY_2 0b0000
#define APIC_TIMER_DIVIDE_BY_4 0b0001
#define APIC_TIMER_DIVIDE_BY_8 0b0010
#define APIC_TIMER_DIVIDE_BY_16 0b0011
#define APIC_TIMER_DIVIDE_BY_32 0b1000
#define APIC_TIMER_DIVIDE_BY_64 0b1001
#define APIC_TIMER_DIVIDE_BY_128 0b1010

ISR_DEFINE(spurious, 0) {
  printf("#SPURIOUS\n");
}

ISR_DEFINE(timer, 0) {
  printf("#TIMER\n");
}

static inline void reg_write(uint64_t reg, uint64_t value) {
  printf("reg %X (prev: %X, curr: %X)\n",
         reg, *(uint64_t*)(APIC_BASE_ADDR + reg), value);
  *(uint64_t*)(APIC_BASE_ADDR + reg) = value;
}

void init_apic(void) {
  uint64_t prev = rdmsr(APIC_BASE_MSR);
  wrmsr(APIC_BASE_MSR, APIC_BASE_ADDR | APIC_ENABLE);
  printf("msr %X (prev: %X, curr: %X)\n",
         APIC_BASE_MSR, prev, rdmsr(APIC_BASE_MSR));

  reg_write(0x30, 0);

  set_isr(INT_VECTOR_SPURIOUS, spurious_isr_getter());
  reg_write(APIC_SPURIOUS_REG, APIC_LOCAL_ENABLE | INT_VECTOR_SPURIOUS);

  set_isr(INT_VECTOR_TIMER, timer_isr_getter());
  reg_write(APIC_TIMER_REG, APIC_TIMER_PERIODIC | INT_VECTOR_TIMER);
  reg_write(APIC_TIMER_INITIAL_REG, 12345);
  reg_write(APIC_TIMER_DIVIDE_REG, APIC_TIMER_DIVIDE_BY_4);
}
