/*
 * x86-64-specific CPU constants and structures.
 */

#ifndef __X86_64_CPU_H_
#define __X86_64_CPU_H_

/* CR0 register bits */
#define CR0_PE (1 << 0)
#define CR0_MP (1 << 1)
#define CR0_EM (1 << 2)
#define CR0_TS (1 << 3)
#define CR0_ET (1 << 4)
#define CR0_NE (1 << 5)
#define CR0_WP (1 << 16)
#define CR0_AM (1 << 18)
#define CR0_NW (1 << 29)
#define CR0_CD (1 << 30)
#define CR0_PG (1 << 31)

/* CR4 register bits */
#define CR4_VME (1 << 0)
#define CR4_PVI (1 << 1)
#define CR4_TSD (1 << 2)
#define CR4_DE (1 << 3)
#define CR4_PSE (1 << 4)
#define CR4_PAE (1 << 5)
#define CR4_MCE (1 << 6)
#define CR4_PGE (1 << 7)
#define CR4_PCE (1 << 8)
#define CR4_OSFXSR (1 << 9)
#define CR4_OSXMMEXCPT (1 << 10)
#define CR4_VMXE (10 << 13)
#define CR4_SmXE (10 << 14)
#define CR4_PCIDE (1 << 17)
#define CR4_OSXSAVE (1 << 18)
#define CR4_SMEP (1 << 20)
#define CR4_SMAP (1 << 21)

/* MSR registers */
#define MSR_EFER 0xC0000080

/* MSR EFER register bits */
#define MSR_EFER_LME (1 << 8)
#define MSR_EFER_LMA (1 << 10)
#define MSR_EFER_NXE (1 << 11)
#define MSR_EFER_SVME (1 << 12)
#define MSR_EFER_LMSLE (1 << 13)
#define MSR_EFER_FXSR (1 << 14)
#define MSR_EFER_TCE (1 << 15)

/* GDT entry types */
#define GDT_DATA 0x2
#define GDT_TSS  0x9
#define GDT_CODE 0xA

/* segment values */
#define SEGMENT_CODE 0x8
#define SEGMENT_DATA 0x10

#define PAGE_TABLE_ENTRIES 512

#ifndef __ASSEMBLER__

#include "std.h"

struct page_table_entry {
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

struct gdt_entry {
  uint16_t limit0;
  uint16_t base0;
  uint32_t base1 : 8;
  uint32_t type : 4;
  uint32_t nonsys : 1;
  uint32_t dpl : 2;
  uint32_t present : 1;
  uint32_t limit1 : 4;
  uint32_t avl : 1;
  uint32_t bits64 : 1;
  uint32_t bits32 : 1;
  uint32_t gran : 1;
  uint32_t base2 : 8;
};

struct gdt_ex_entry {
  struct gdt_entry entry;
  uint32_t base3;
  uint32_t reserved0 : 8;
  uint32_t zero : 5;
  uint32_t reserved1 : 19;
};

struct gdt {
  struct gdt_entry zero;
  struct gdt_entry code;
  struct gdt_entry data;
  struct gdt_ex_entry cpus[MAX_CPUS];
};

struct desc_table_info {
  uint16_t limit;
  uint64_t base;
} __attribute__((packed));

#endif /* __ASSEMBLER__ */

#endif /* __X86_64_CPU_H_ */
