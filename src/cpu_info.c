#include "acpi.h"
#include "config.h"
#include "cpu_info.h"

#define VENDOR_LEN 12

static int vendor = 0, num = 0, bsp = 0;
static struct cpu_desc descs[CONFIG_CPUS_MAX];
static uint8_t indexes[256];

int get_cpu_index(void) {
  uint32_t ebx;
  ASMV("movl $1, %%eax\ncpuid" : "=b"(ebx) : : "eax", "ecx", "edx");
  return indexes[INT_BITS(ebx, 24, 31)];
}

int get_bsp_cpu_index(void) {
  return bsp;
}

int get_cpu_vendor(void) {
  if (vendor)
    return vendor;
  else {
    uint32_t buf[4] = { };
    ASMV("xorl %%eax, %%eax\ncpuid" :
         "=b"(buf[0]), "=d"(buf[1]), "=c"(buf[2]) : : "eax");
    if (!memcmp(buf, "GenuineIntel", VENDOR_LEN))
      vendor = CPU_VENDOR_INTEL;
    else if (!memcmp(buf, "AuthenticAMD", VENDOR_LEN))
      vendor = CPU_VENDOR_AMD;
    else
      vendor = CPU_VENDOR_UNKNOWN;
    LOG_DEBUG("vendor %d (%s)", vendor, (char*)buf);
    return vendor;
  }
}

int get_cpus(void) {
  return num;
}

const struct cpu_desc *get_cpu_desc(int index) {
  return &descs[index];
}

static bool get_chip_multithreading(int *chip_threads_max) {
  uint32_t ebx, edx;
  ASMV("movl $1, %%eax\ncpuid" : "=b"(ebx), "=d"(edx) : : "eax", "ecx");
  bool support = !!(edx & (1 << 28));
  *chip_threads_max = INT_BITS(ebx, 16, 23);
  LOG_DEBUG("support: %s, chip_threads_max: %d", bool_str(support),
            *chip_threads_max);
  return support;
}

static void get_thread_core_bits_intel(uint32_t cpuid_func_max,
                                       UNUSED uint32_t cpuid_ex_func_max,
                                       int chip_threads_max, int *thread_bits,
                                       int *core_bits) {
  uint32_t eax;
  if (cpuid_func_max >= 0xB) {
    ASMV("movl $0xB, %%eax\nxorl %%ecx, %%ecx\ncpuid" :
         "=a"(eax) : : "ebx", "ecx", "edx");
    *thread_bits = INT_BITS(eax, 0, 4);
    ASMV("movl $0xB, %%eax\nmovl $1, %%ecx\ncpuid" :
         "=a"(eax) : : "ebx", "ecx", "edx");
    *core_bits = INT_BITS(eax, 0, 4) - *thread_bits;
  }
  else if (cpuid_func_max >= 0x4) {
    ASMV("movl $0x4, %%eax\nxorl %%ecx, %%ecx\ncpuid" :
         "=a"(eax) : : "ebx", "ecx", "edx");
    int core_index_max = INT_BITS(eax, 26, 31);
    *core_bits = core_index_max ? bsr(core_index_max) + 1 : 0;
    int chip_thread_bits = bsr(chip_threads_max - 1) + 1;
    *thread_bits = chip_thread_bits - *core_bits;
  }
  else
    *core_bits = 0, *thread_bits = bsr(chip_threads_max - 1) + 1;
}

static void get_thread_core_bits_amd(UNUSED uint32_t cpuid_func_max,
                                     uint32_t cpuid_ex_func_max,
                                     int chip_threads_max, int *thread_bits,
                                     int *core_bits) {
  if (cpuid_ex_func_max >= 0x80000008) {
    uint32_t ecx;
    ASMV("movl $0x80000008, %%eax\ncpuid" : "=c"(ecx) : : "eax", "ebx", "edx");
    *core_bits = INT_BITS(ecx, 12, 15);
    if (!*core_bits) {
      int core_index_max = INT_BITS(ecx, 0, 7);
      *core_bits = core_index_max ? bsr(core_index_max) + 1 : 0;
    }
    int chip_thread_bits = bsr(chip_threads_max - 1) + 1;
    *thread_bits = chip_thread_bits - *core_bits;
  }
  else
    *core_bits = bsr(chip_threads_max - 1) + 1, *thread_bits = 0;
}

static void get_thread_core_bits(int *thread_bits, int *core_bits) {
  uint32_t cpuid_func_max, cpuid_ex_func_max;
  ASMV("xorl %%eax, %%eax\ncpuid" :
       "=a"(cpuid_func_max) : : "ebx", "ecx", "edx");
  ASMV("movl $0x80000000, %%eax\ncpuid" :
       "=a"(cpuid_ex_func_max) : : "ebx", "ecx", "edx");

  int chip_threads_max;
  if (get_cpu_vendor() != CPU_VENDOR_UNKNOWN &&
      get_chip_multithreading(&chip_threads_max)) {
    if (get_cpu_vendor() == CPU_VENDOR_INTEL)
      get_thread_core_bits_intel(cpuid_func_max, cpuid_ex_func_max,
                                 chip_threads_max, thread_bits, core_bits);
    else
      get_thread_core_bits_amd(cpuid_func_max, cpuid_ex_func_max,
                               chip_threads_max, thread_bits, core_bits);
    LOG_DEBUG("thread_bits: %d, core_bits: %d", *thread_bits, *core_bits);
  }
  else
    *thread_bits = *core_bits = 0;
}

void fill_cpu_descs(int thread_bits, int core_bits) {
  int i = 0;
  struct acpi_madt_lapic *mentry = NULL;
  while (get_next_acpi_entry(get_acpi_madt(), &mentry, ACPI_MADT_LAPIC_TYPE))
    if (mentry->enabled) {
      if (i >= CONFIG_CPUS_MAX) {
        LOG_ERROR("detected more than %s=%d cpus",
                  STR(CONFIG_CPUS_MAX), CONFIG_CPUS_MAX);
        break;
      }

      indexes[mentry->apic_id] = i;
      descs[i].apic_id = mentry->apic_id;
      descs[i].thread = mentry->apic_id & ((1 << thread_bits) - 1);
      descs[i].core = (mentry->apic_id >> thread_bits) &
        ((1 << core_bits) - 1);
      descs[i].chip = mentry->apic_id >> (thread_bits + core_bits);

      struct acpi_srat_lapic *sentry = NULL;
      while (get_next_acpi_entry(get_acpi_srat(), &sentry,
                                 ACPI_SRAT_LAPIC_TYPE))
        if (sentry->enabled && sentry->apic_id == mentry->apic_id)
          descs[i].domain = sentry->prox_domain0 +
            ((uint32_t)sentry->prox_domain1 << 8) +
            ((uint32_t)sentry->prox_domain2 << 24);

      LOG_DEBUG("CPU: apic_id: %X, thread: %d, core: %d, chip: %d, domain: %X",
                descs[i].apic_id, descs[i].thread, descs[i].core,
                descs[i].chip, descs[i].domain);
      i++;
    }

  num = i;
}

void init_cpu_info(void) {
  int thread_bits, core_bits;
  get_thread_core_bits(&thread_bits, &core_bits);
  fill_cpu_descs(thread_bits, core_bits);
  bsp = get_cpu_index();
  LOG_DEBUG("done");
}
