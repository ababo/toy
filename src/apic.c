#include "addr_map.h"
#include "apic.h"
#include "interrupt.h"
#include "page_map.h"

#define LAPIC_BASE_MSR 0x1B

#define LAPIC_ID_REG 0x20
#define LAPIC_EOI_REG 0xB0
#define LAPIC_SPURIOUS_REG 0xF0
#define LAPIC_TIMER_REG 0x320
#define LAPIC_TIMER_INITIAL_REG 0x380
#define LAPIC_TIMER_CURRENT_REG 0x390
#define LAPIC_TIMER_DIVIDE_REG 0x3E0

#define LAPIC_ENABLE (1 << 11)
#define LAPIC_LOCAL_ENABLE (1 << 8)
#define LAPIC_TIMER_MASKED (1 << 16)
#define LAPIC_TIMER_PERIODIC (1 << 17)

#define LAPIC_TIMER_DIVIDE_BY_1 0b1011
#define LAPIC_TIMER_DIVIDE_BY_2 0b0000
#define LAPIC_TIMER_DIVIDE_BY_4 0b0001
#define LAPIC_TIMER_DIVIDE_BY_8 0b0010
#define LAPIC_TIMER_DIVIDE_BY_16 0b0011
#define LAPIC_TIMER_DIVIDE_BY_32 0b1000
#define LAPIC_TIMER_DIVIDE_BY_64 0b1001
#define LAPIC_TIMER_DIVIDE_BY_128 0b1010

static uint32_t timer_10ms_ticks;

static inline uint32_t read_lapic(int reg) {
  return *(volatile uint32_t*)((uint64_t)ADDR_LOCAL_APIC + reg);
}

static inline void write_lapic(int reg, uint32_t value) {
  *(volatile uint32_t*)((uint64_t)ADDR_LOCAL_APIC + reg) = value;
}

int get_apic_id(void) {
  return INT_BITS(read_lapic(LAPIC_ID_REG), 24, 31);
}

bool is_bootstrap_cpu(void) {
  return !!(rdmsr(LAPIC_BASE_MSR) & (1 << 8));
}

void set_apic_eoi(void) {
  write_lapic(LAPIC_EOI_REG, 0);
}

void start_apic_timer(int interval, bool periodic) {
  stop_apic_timer();

  uint64_t initial = (uint64_t)interval * timer_10ms_ticks / 10000;
  int extra_bits = bsr(initial) - 31, divide;
  switch (extra_bits) {
  case 1: initial >>= 1, divide = LAPIC_TIMER_DIVIDE_BY_2; break;
  case 2: initial >>= 2, divide = LAPIC_TIMER_DIVIDE_BY_4; break;
  case 3: initial >>= 3, divide = LAPIC_TIMER_DIVIDE_BY_8; break;
  case 4: initial >>= 4, divide = LAPIC_TIMER_DIVIDE_BY_16; break;
  case 5: initial >>= 5, divide = LAPIC_TIMER_DIVIDE_BY_32; break;
  case 6: initial >>= 6, divide = LAPIC_TIMER_DIVIDE_BY_64; break;
  case 7: initial >>= 7, divide = LAPIC_TIMER_DIVIDE_BY_128; break;
  default:
    if (extra_bits > 7) {
      LOG_ERROR("timer overflow");
      return;
    }
    divide = LAPIC_TIMER_DIVIDE_BY_1;
    if (!initial)
      initial = 1;
  }

  write_lapic(LAPIC_TIMER_REG,
              (periodic ? LAPIC_TIMER_PERIODIC : 0) | INT_VECTOR_APIC_TIMER);
  write_lapic(LAPIC_TIMER_DIVIDE_REG, divide);
  write_lapic(LAPIC_TIMER_INITIAL_REG, (uint32_t)initial);
}

void stop_apic_timer(void) {
  write_lapic(LAPIC_TIMER_INITIAL_REG, 0);
}

ISR_DEFINE(empty, 0) {
  set_apic_eoi();
}

static void init_timer(void) {
  set_isr(INT_VECTOR_APIC_TIMER, empty_isr_getter());
  write_lapic(LAPIC_TIMER_REG, INT_VECTOR_APIC_TIMER);
  write_lapic(LAPIC_TIMER_DIVIDE_REG, LAPIC_TIMER_DIVIDE_BY_1);

  outb(0x61, (inb(0x61) & 0x0C) | 1); // enable PIT
  outb(0x43, 0xB0);

  write_lapic(LAPIC_TIMER_INITIAL_REG, UINT32_MAX); // wait for 10 ms
  outb(0x42, 0x9C);
  outb(0x42, 0x2E);
  while (!(inb(0x61) & 0x20));

  timer_10ms_ticks = UINT32_MAX - read_lapic(LAPIC_TIMER_CURRENT_REG);
  write_lapic(LAPIC_TIMER_INITIAL_REG, 0);

  outb(0x61, inb(0x61) & 0x0C); // disable PIT
}

void init_apic(void) {
  wrmsr(LAPIC_BASE_MSR, ADDR_LOCAL_APIC | LAPIC_ENABLE);
  map_page(ADDR_LOCAL_APIC, ADDR_LOCAL_APIC,
           PAGE_MAPPING_WRITE | PAGE_MAPPING_PWT | PAGE_MAPPING_PCD, 0);
  set_isr(INT_VECTOR_APIC_SPURIOUS, empty_isr_getter());
  write_lapic(LAPIC_SPURIOUS_REG, LAPIC_LOCAL_ENABLE |
              INT_VECTOR_APIC_SPURIOUS);
  init_timer();
  LOG_DEBUG("done");
}
