#include "acpi.h"
#include "cpu_info.h"

#define CPUS_NUMBER_MAX 32
#define VENDOR_STR_LENGTH 12

static int vendor, num, bsp;
static struct cpu_desc info[CPUS_NUMBER_MAX];

int get_cpu_vendor(void) {
  if (vendor)
    return vendor;
  else {
    uint32_t buf[4] = { };
    asm("xorl %%eax, %%eax\ncpuid" :
        "=b"(buf[0]), "=d"(buf[1]), "=c"(buf[2]) : : "eax");
    if (!memcmp(buf, "GenuineIntel", VENDOR_STR_LENGTH))
      vendor = CPU_VENDOR_INTEL;
    else if (!memcmp(buf, "AuthenticAMD", VENDOR_STR_LENGTH))
      vendor = CPU_VENDOR_AMD;
    else
      vendor = CPU_VENDOR_UNKNOWN;
    LOG_DEBUG("vendor %d (%s)", vendor, buf);
    return vendor;
  }
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

static bool get_chip_multithreading(int *threads_number_max) {
  uint32_t ebx, edx;
  asm("movl $1, %%eax\ncpuid" : "=b"(ebx), "=d"(edx) : : "eax");
  bool support = !!(edx & (1 << 28));
  *threads_number_max = INT_BITS(ebx, 16, 23);
  LOG_DEBUG("support: %s, threads_number_max: %d", bool_str(support),
            *threads_number_max);
  return support;
}

static void get_bits_number_intel(uint32_t cpuid_func_max,
                                  uint32_t cpuid_ex_func_max,
                                  int chip_threads_number_max,
                                  int *thread_bits_number,
                                  int *core_bits_number) {
  if (cpuid_func_max >= 0xB) {
    uint32_t eax;
    asm("movl $0xB, %%eax\nxorl %%ecx, %%ecx\ncpuid" : "=a"(eax));
    *thread_bits_number = INT_BITS(eax, 0, 4);
    asm("movl $0xB, %%eax\nmovl $1, %%ecx\ncpuid" : "=a"(eax));
    *core_bits_number = INT_BITS(eax, 0, 4);
  }
  else {
    printf("intel func unsupported\n");
  }
}

static void get_bits_number_amd(uint32_t cpuid_func_max,
                                uint32_t cpuid_ex_func_max,
                                int chip_threads_number_max,
                                int *thread_bits_number,
                                int *core_bits_number) {
  if (cpuid_ex_func_max >= 0x80000008) {
    uint32_t ecx;
    asm("movl $0x80000008, %%eax\ncpuid" : "=c"(ecx) : : "eax");
    *core_bits_number = INT_BITS(ecx, 12, 15);
    if (!*core_bits_number) {
    int core_index_max = INT_BITS(ecx, 0, 7);
    *core_bits_number = core_index_max ? bsr(core_index_max) + 1 : 0;
    }

  }
  else {
    printf("amd func unsupported\n");
  }
}

void init_cpu_info(void) {
  uint32_t cpuid_func_max, cpuid_ex_func_max;
  asm("xorl %%eax, %%eax\ncpuid" : "=a"(cpuid_func_max));
  asm("movl $0x80000000, %%eax\ncpuid" : "=a"(cpuid_ex_func_max));

  int chip_threads_number_max, thread_bits_number, core_bits_number;
  if (get_cpu_vendor() != CPU_VENDOR_UNKNOWN &&
      get_chip_multithreading(&chip_threads_number_max)) {
    if (get_cpu_vendor() == CPU_VENDOR_INTEL)
      get_bits_number_intel(cpuid_func_max, cpuid_ex_func_max,
                            chip_threads_number_max, &thread_bits_number,
                            &core_bits_number);
    else
      get_bits_number_amd(cpuid_func_max, cpuid_ex_func_max,
                          chip_threads_number_max, &thread_bits_number,
                          &core_bits_number);
    LOG_DEBUG("thread_bits_number: %d, core_bits_number: %d",
              thread_bits_number, core_bits_number);
  }
  else
    thread_bits_number = core_bits_number = 0;

  LOG_DEBUG("done");
}
