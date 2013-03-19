#ifndef SYS_TABLE_H
#define SYS_TABLE_H

#include "util.h"

#define SYS_GDT_DESC_SIZE 8
#define SYS_GDT_DESC2_SIZE 16
#define SYS_IDT_DESC_SIZE 16
#define SYS_PAGE_DESC_SIZE 8

#define SYS_PAGE_DESC_ADDR0_BS 12
#define SYS_PAGE_DESC_ADDR1_BS 32
#define SYS_PAGE_DESC_AVAIL1_BS 3

#define SYS_PAGE_TABLE_DESCS 512
#define SYS_PAGE_TABLE_SIZE 4096

#define SYS_SEGMENT_CODE 0xA
#define SYS_SEGMENT_DATA 0x2
#define SYS_SEGMENT_TSS 0x9
#define SYS_GATE_INT 0xE

struct sys_task_segment {
  uint32_t reserved0;
  uint64_t rsps[3];
  uint64_t reserved1;
  uint64_t ists[7];
  uint64_t reserved2;
  uint16_t reserved3;
  uint16_t iomap;
} PACKED;

struct sys_gdt_desc {
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

struct sys_gdt_desc2 {
  struct sys_gdt_desc base;
  uint32_t base3;
  uint32_t reserved0 : 8;
  uint32_t zero : 5;
  uint32_t reserved : 19;
};

struct sys_idt_desc {
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

struct sys_table_info {
  uint16_t limit;
  uint64_t base;
} PACKED;

struct sys_page_desc {
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

#endif // SYS_TABLE_H
