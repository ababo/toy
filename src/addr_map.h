#ifndef ADDR_MAP_H
#define ADDR_MAP_H

#include "util.h"

#define ADDR_MAP 0x500

#define ADDR_AP_CPU_BOOT 0x1000

#define ADDR_TSS 0x5000
#define ADDR_GDT 0x5080
#define ADDR_IDT 0x5100 // up to 0x6000

#define ADDR_INT_STACK 0x7000 // down to 0x6000
#define ADDR_KMAIN_STACK 0x7C00

#define ADDR_KERNEL 0x7E00

#define ADDR_EBDA 0x9FC00
#define ADDR_AFTER_EBDA 0xA0000

#define ADDR_VIDEO 0xB8000

#define ADDR_BIOS_ROM 0xE0000
#define ADDR_AFTER_BIOS_ROM 0x100000

#define ADDR_PAGE_MAP_TEMP 0x100000
#define ADDR_PAGE_MAP 0x200000

#define ADDR_IO_APIC 0xFEC00000
#define ADDR_LOCAL_APIC 0xFEE00000

#define ADDR_MAP_USABLE 1
#define ADDR_MAP_RESERVED 2
#define ADDR_MAP_ACPI_RECLAIM 3
#define ADDR_MAP_ACPI_NVS 4
#define ADDR_MAP_BAD 5

struct addr_map_entry {
  uint64_t base;
  uint64_t size;
  uint32_t type;
  uint32_t present : 1;
  uint32_t non_volat : 1;
  uint32_t reserved : 30;
};

static inline int get_addr_map_length(void) {
  return *(uint16_t*)ADDR_MAP;
}

static inline const struct addr_map_entry *get_addr_map(void) {
  return (struct addr_map_entry*)(ADDR_MAP + 8);
}

#endif // ADDR_MAP_H
