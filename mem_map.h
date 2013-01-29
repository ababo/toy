#ifndef MEM_MAP_H
#define MEM_MAP_H

#define TSS_ADDR 0x10
#define GDT_ADDR 0x80
#define IDT_ADDR 0x100

#define PML4_ADDR 0x1000
#define PDPT0_ADDR 0x2000
#define PD0_ADDR 0x3000
#define PT0_ADDR 0x4000

#define APIC_BASE_ADDR 0x5000

#define INT_STACK_ADDR 0x6C00
#define KMAIN_STACK_ADDR 0x7C00

#define KERNEL_ADDR 0x7E00

#define VIDEO_ADDR 0xB8000

#endif // MEM_MAP_H
