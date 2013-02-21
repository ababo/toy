#include "acpi.h"
#include "schedule.h"

#define CPUS_NUMBER_MAX 32

static int cpus_number;
static int bsp_cpu;

static struct {
  uint8_t apic_id;
  uint8_t domain;
  uint8_t chip;
  uint8_t core;
  uint8_t thread;
} cpus[CPUS_NUMBER_MAX];

static void detect_cpus(void) {

}

void init_scheduler(void) {
  detect_cpus();

}
