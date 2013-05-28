#ifndef __X86_64_CPU_H
#define __X86_64_CPU_H

#include "../common.h"

#define CR0_PG (1 << 31)
#define CR0_PE (1 << 0)
#define CR4_OSFXSR (1 << 9)
#define CR4_PAE (1 << 5)
#define MSR_EFER 0xC0000080
#define MSR_EFER_LME (1 << 8)
#define MSR_FS_BASE 0xC0000100
#define MSR_GS_BASE 0xC0000101

#define SEGMENT_CODE (1 << 3)
#define SEGMENT_DATA (2 << 3)

#define RFLAGS_IF (1 << 9)

#define GDT_DESC_SIZE 8
#define GDT_DESC2_SIZE 16
#define IDT_DESC_SIZE 16
#define PAGE_DESC_SIZE 8

#define PAGE_DESC_ADDR0_BS 12
#define PAGE_DESC_ADDR1_BS 32
#define PAGE_DESC_AVAIL1_BS 3

#define PAGE_TABLE_DESCS 512
#define PAGE_TABLE_SIZE 4096

#define GDT_SEGMENT_CODE 0xA
#define GDT_SEGMENT_DATA 0x2
#define GDT_SEGMENT_TSS 0x9
#define IDT_GATE_INT 0xE

struct task_segment {
  uint32_t reserved0;
  uint64_t rsps[3];
  uint64_t reserved1;
  uint64_t ists[7];
  uint64_t reserved2;
  uint16_t reserved3;
  uint16_t iomap;
} PACKED;

struct gdt_desc {
  uint16_t limit0;
  uint16_t base0;
  uint8_t base1;
  uint8_t type : 4;
  uint8_t nonsys : 1;
  uint8_t dpl : 2;
  uint8_t present : 1;
  uint8_t limit1 : 4;
  uint8_t avl : 1;
  uint8_t bits64 : 1;
  uint8_t bits32 : 1;
  uint8_t gran : 1;
  uint8_t base2;
};

struct gdt_desc2 {
  struct gdt_desc base;
  uint32_t base3;
  uint32_t reserved0 : 8;
  uint32_t zero : 5;
  uint32_t reserved : 19;
};

struct idt_desc {
  uint16_t handler0;
  uint16_t cs;
  uint16_t ist : 3;
  uint16_t reserved0 : 5;
  uint16_t type : 4;
  uint16_t zero : 1;
  uint16_t dpl : 2;
  uint16_t present : 1;
  uint16_t handler1;
  uint32_t handler2;
  uint32_t reserved1;
};

struct cpu_table_info {
  uint16_t limit;
  uint64_t base;
} PACKED;

struct page_desc {
  uint32_t present : 1;
  uint32_t write : 1;
  uint32_t nonsys : 1;
  uint32_t pwt : 1;
  uint32_t pcd : 1;
  uint32_t accessed : 1;
  uint32_t dirty : 1;
  uint32_t ps_pat : 1;
  uint32_t global : 1;
  uint32_t avail0 : 3;
  uint32_t address0 : 20;
  uint32_t address1 : 20;
  uint32_t avail1 : 11;
  uint32_t noexec : 1;
};

static inline int bsrq(uint64_t value) {
  uint64_t index;
  ASMV("bsrq %1, %0" : "=r"(index) : "mr"(value));
  return (int)index;
}

static inline int bsfq(uint64_t value) {
  uint64_t index;
  ASMV("bsfq %1, %0" : "=r"(index) : "mr"(value));
  return (int)index;
}

static inline uint8_t inb(uint16_t port) {
  uint8_t value;
  ASMV("inb %%dx, %%al" : "=a"(value) : "d"(port));
  return value;
}

static inline uint16_t inw(uint16_t port) {
  uint16_t value;
  ASMV("inw %%dx, %%ax" : "=a"(value) : "d"(port));
  return value;
}

static inline uint32_t inl(uint16_t port) {
  uint32_t value;
  ASMV("inl %%dx, %%eax" : "=a"(value) : "d"(port));
  return value;
}

static inline void outb(uint16_t port, uint8_t value) {
  ASMV("outb %%al, %%dx" : : "a"(value), "d"(port));
}

static inline void outw(uint16_t port, uint16_t value) {
  ASMV("outw %%ax, %%dx" : : "a"(value), "d"(port));
}

static inline void outl(uint16_t port, uint32_t value) {
  ASMV("outl %%eax, %%dx" : : "a"(value), "d"(port));
}

static inline uint64_t rdmsr(uint32_t msr) {
  uint32_t low, high;
  ASMV("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
  return ((uint64_t)high << 32) + low;
}

static inline void wrmsr(uint32_t msr, uint64_t value) {
  uint32_t low = (uint32_t)value, high = (uint32_t)(value >> 32);
  ASMV("wrmsr" : : "a"(low), "d"(high), "c"(msr));
}

static inline uint64_t cmpxchgq(uint64_t *ptr, uint64_t old, uint64_t new) {
  uint64_t ret;
  ASMV("lock\ncmpxchgq %1, %2"
       : "=a"(ret) : "r"(new), "m"(*ptr), "0"(old) : "memory");
  return ret;
}

#endif // __X86_64_CPU_H
