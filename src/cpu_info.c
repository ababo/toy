#include "acpi.h"
#include "cpu_info.h"

#define CPUS_NUMBER_MAX 32
#define VENDOR_STR_LENGTH 12

static int num, bsp;
static struct cpu_desc info[CPUS_NUMBER_MAX];

int get_cpu_vendor(void) {
  uint32_t vendor[3];
  asm("xorl %%eax, %%eax\ncpuid" :
      "=b"(vendor[0]), "=d"(vendor[1]), "=c"(vendor[2]) : : "eax");
  if (!memcmp(vendor, "GenuineIntel", VENDOR_STR_LENGTH))
    return CPU_VENDOR_INTEL;
  if (!memcmp(vendor, "AuthenticAMD", VENDOR_STR_LENGTH))
    return CPU_VENDOR_AMD;
  return CPU_VENDOR_UNKNOWN;
}

int get_cpus_number(void) {
  return num;
}

int get_cpu_bsp(void) {
  return bsp;
}

const struct cpu_desc *get_cpu_info(void) {
  return info;
}

void init_cpu_info(void) {

  LOG_DEBUG("init_cpu_info: done");
}
