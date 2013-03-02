#include "addr_map.h"
#include "apic.h"
#include "schedule.h"

static void set_ap_cpu_jump(void) {
  extern int apboot;
  uint16_t segment = (uint16_t)INT_BITS((uint64_t)&apboot, 4, 19);
  uint16_t offset = (uint16_t)INT_BITS((uint64_t)&apboot, 0, 3);

  // encoding for ljmp $segment, $offset
  *(uint8_t*)ADDR_AP_CPU_BOOT = 0xEA;
  *(uint16_t*)(ADDR_AP_CPU_BOOT + 1) = offset;
  *(uint16_t*)(ADDR_AP_CPU_BOOT + 3) = segment;
}

void init_scheduler(void) {
  set_ap_cpu_jump();
  start_ap_cpu(2, ADDR_AP_CPU_BOOT);
  LOG_DEBUG("done");
}
