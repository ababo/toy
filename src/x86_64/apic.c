#include "../cpu_info.h"
#include "apic.h"
#include "interrupt.h"
#include "page_map.h"

#define LAPIC_ADDR 0xFEE00000
#define LAPIC_BASE_MSR 0x1B

#define LAPIC_ID_REG 0x20
#define LAPIC_EOI_REG 0xB0
#define LAPIC_SPURIOUS_REG 0xF0
#define LAPIC_ICR_LOW_REG 0x300
#define LAPIC_ICR_HIGH_REG 0x310
#define LAPIC_TIMER_REG 0x320
#define LAPIC_TIMER_INITIAL_REG 0x380
#define LAPIC_TIMER_CURRENT_REG 0x390
#define LAPIC_TIMER_DIVIDE_REG 0x3E0

#define LAPIC_ENABLE (1 << 11)
#define LAPIC_LOCAL_ENABLE (1 << 8)
#define LAPIC_TIMER_MASKED (1 << 16)
#define LAPIC_TIMER_PERIODIC (1 << 17)

#define LAPIC_TIMER_DIVIDE_BY_1 0xB
#define LAPIC_TIMER_DIVIDE_BY_2 0x0
#define LAPIC_TIMER_DIVIDE_BY_4 0x1
#define LAPIC_TIMER_DIVIDE_BY_8 0x2
#define LAPIC_TIMER_DIVIDE_BY_16 0x3
#define LAPIC_TIMER_DIVIDE_BY_32 0x8
#define LAPIC_TIMER_DIVIDE_BY_64 0x9
#define LAPIC_TIMER_DIVIDE_BY_128 0xA

#define LAPIC_ICR_DELIVERY_FIXED (0x0 << 8)
#define LAPIC_ICR_DELIVERY_LOWEST_PRIORITY (0x1 << 8)
#define LAPIC_ICR_DELIVERY_SMI (0x2 << 8)
#define LAPIC_ICR_DELIVERY_NMI (0x4 << 8)
#define LAPIC_ICR_DELIVERY_INIT (0x5 << 8)
#define LAPIC_ICR_DELIVERY_STARTUP (0x6 << 8)

#define LAPIC_ICR_DESTINATION_PHYSICAL (0 << 11)
#define LAPIC_ICR_DESTINATION_LOGICAL (1 << 11)

#define LAPIC_ICR_STATUS_IDLE (0 << 12)
#define LAPIC_ICR_STATUS_SEND_PENDING (1 << 12)

#define LAPIC_ICR_LEVEL_DEASSERT (0 << 14)
#define LAPIC_ICR_LEVEL_ASSERT (1 << 14)

#define LAPIC_ICR_TRIGGER_EDGE (0 << 15)
#define LAPIC_ICR_TRIGGER_LEVEL (1 << 15)

#define LAPIC_ICR_SHORTHAND_NONE (0x0 << 18)
#define LAPIC_ICR_SHORTHAND_SELF (0x1 << 18)
#define LAPIC_ICR_SHORTHAND_ALL (0x2 << 18)
#define LAPIC_ICR_SHORTHAND_OTHER (0x3 << 18)

#define AP_CPU_INIT_TIMEOUT 20000
#define AP_CPU_RETRY_TIMEOUT 20000
#define AP_CPU_STARTUP_TIMEOUT 200000

static uint32_t timer_10ms_ticks;

static inline uint32_t read_lapic(int reg) {
  return *(volatile uint32_t*)((uint64_t)LAPIC_ADDR + reg);
}

static inline void write_lapic(int reg, uint32_t value) {
  *(volatile uint32_t*)((uint64_t)LAPIC_ADDR + reg) = value;
}

void start_apic_timer(int interval, bool periodic) {
  stop_apic_timer();

  uint64_t initial = (uint64_t)interval * timer_10ms_ticks / 10000;
  int extra_bits = bsrq(initial) - 31, divide;
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
      ASMV("jmp halt");
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

void issue_cpu_interrupt(int cpu, int vector) {
  write_lapic(LAPIC_ICR_HIGH_REG, get_cpu_desc(cpu)->id << 24);
  write_lapic(LAPIC_ICR_LOW_REG, (uint8_t)vector |
              LAPIC_ICR_DELIVERY_FIXED | LAPIC_ICR_LEVEL_ASSERT);
}

bool start_ap_cpu(int cpu, int startup_addr, volatile int *started_cpus) {
  int apic_id = get_cpu_desc(cpu)->id;
  write_lapic(LAPIC_ICR_HIGH_REG, apic_id << 24);
  write_lapic(LAPIC_ICR_LOW_REG, LAPIC_ICR_DELIVERY_INIT |
              LAPIC_ICR_LEVEL_ASSERT);
  start_apic_timer(AP_CPU_INIT_TIMEOUT, false);
  ASMV("hlt");

  for (int cpus = *started_cpus, try = 0; try < 2; try++) {
    write_lapic(LAPIC_ICR_HIGH_REG, apic_id << 24);
    write_lapic(LAPIC_ICR_LOW_REG, (uint8_t)(startup_addr >> 12) |
                LAPIC_ICR_DELIVERY_STARTUP | LAPIC_ICR_LEVEL_ASSERT);

    for (int timeout = 0; timeout < AP_CPU_STARTUP_TIMEOUT;
         timeout += AP_CPU_RETRY_TIMEOUT) {
      start_apic_timer(AP_CPU_RETRY_TIMEOUT, false);
      ASMV("hlt");

      if (cpus < *started_cpus)
        return true;
    }
  }

  return false;
}

DEFINE_ISR(empty, 0) {
  set_apic_eoi();
}

static void adjust_lapic_timer(void) {
  outb(0x61, (inb(0x61) & 0x0C) | 1); // enable PIT
  outb(0x43, 0xB0);

  write_lapic(LAPIC_TIMER_INITIAL_REG, UINT32_MAX); // wait for 10 ms
  outb(0x42, 0x9C);
  outb(0x42, 0x2E);
  while (!(inb(0x61) & 0x20));

  timer_10ms_ticks = UINT32_MAX - read_lapic(LAPIC_TIMER_CURRENT_REG);
  write_lapic(LAPIC_TIMER_INITIAL_REG, 0);

  outb(0x61, inb(0x61) & 0x0C); // disable PIT
  LOG_DEBUG("10ms ticks: %d", timer_10ms_ticks);
}

void init_apic(void) {
  int cpu = get_cpu();
  if (cpu == get_bsp_cpu()) {
    map_page(LAPIC_ADDR, LAPIC_ADDR,
             PAGE_MAPPING_WRITE | PAGE_MAPPING_PWT | PAGE_MAPPING_PCD, 0);
    set_isr(INT_VECTOR_APIC_SPURIOUS, get_empty_isr());
    set_isr(INT_VECTOR_APIC_TIMER, get_empty_isr());
  }

  wrmsr(LAPIC_BASE_MSR, LAPIC_ADDR | LAPIC_ENABLE);
  write_lapic(LAPIC_SPURIOUS_REG, LAPIC_LOCAL_ENABLE |
              INT_VECTOR_APIC_SPURIOUS);
  write_lapic(LAPIC_TIMER_REG, INT_VECTOR_APIC_TIMER);
  write_lapic(LAPIC_TIMER_DIVIDE_REG, LAPIC_TIMER_DIVIDE_BY_1);

  if (cpu == get_bsp_cpu()) {
    adjust_lapic_timer();
  }

  LOG_DEBUG("done (CPU: %d)", cpu);
}
