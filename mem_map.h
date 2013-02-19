#ifndef MEM_MAP_H
#define MEM_MAP_H

#include "util.h"

#define MEMORY_MAP_ADDR 0x500

#define TSS_ADDR 0x5000
#define GDT_ADDR 0x5080
#define IDT_ADDR 0x5100 // up to 0x6000

#define INT_STACK_ADDR 0x7000 // down to 0x6000
#define KMAIN_STACK_ADDR 0x7C00

#define KERNEL_ADDR 0x7E00

#define EBDA_ADDR 0x9FC00
#define EBDA_NEXT_ADDR 0xA0000

#define VIDEO_ADDR 0xB8000

#define BIOS_ROM_ADDR 0xE0000
#define BIOS_ROM_NEXT_ADDR 0x100000

#define PAGE_MAP_TEMP_ADDR 0x100000

#define PAGE_MAP_ADDR 0x200000

#define APIC_BASE_ADDR 0xFEE00000

uint64_t get_usable_memory_size(void);

#endif // MEM_MAP_H
