#include "desc_table.h"
#include "interrupt.h"
#include "mem_map.h"
#include "util.h"

#define CODE_SEGMENT_TYPE 0b1010
#define DATA_SEGMENT_TYPE 0b0010
#define TSS_SEGMENT_TYPE 0b1001
#define INT_GATE_TYPE 0b1110

struct task_segment {
  uint32_t reserved0;
  uint64_t rsps[3];
  uint64_t reserved1;
  uint64_t ists[7];
  uint64_t reserved2;
  uint16_t reserved3;
  uint16_t iomap;
} __attribute__((packed));

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

struct desc_table_info {
  uint16_t limit;
  uint64_t base;
} __attribute__((packed));

static void create_gdt(void) {
  struct task_segment *tss = (struct task_segment*)TSS_ADDR;
  *tss = (struct task_segment) { .ists = { INT_STACK_ADDR } };

  uint64_t *gdt = (uint64_t*)GDT_ADDR;
  *(struct gdt_desc*)gdt++ = (struct gdt_desc) {};
  *(struct gdt_desc*)gdt++ = (struct gdt_desc) {
    .type = CODE_SEGMENT_TYPE, .nonsys = true, .present = true, .bits64 = true
  };
  *(struct gdt_desc*)gdt++ = (struct gdt_desc) {
    .type = DATA_SEGMENT_TYPE, .nonsys = true, .present = true, .bits32 = true
  };
  *(struct gdt_desc*)gdt++ = (struct gdt_desc) {
    .type = TSS_SEGMENT_TYPE, .nonsys = false, .present = true,
    .base0 = TSS_ADDR, .limit0 = sizeof(struct task_segment) - 1
  };

  struct desc_table_info gdti = { (8 * 3) + (16 * 1) - 1, GDT_ADDR };
  __asm__("lgdt %0\nmovw $(8 * 3), %%ax\nltr %%ax" : : "m"(gdti));
}

static void create_idt(void) {
  struct idt_desc *idt = (struct idt_desc*)IDT_ADDR;
  for (int i = 0; i < INT_VECTOR_NUMBER; i++)
    if (!is_int_reserved(i))
      idt[i] = (struct idt_desc) {
        .cs = 8, .ist = 1, .type = INT_GATE_TYPE, .present = false
      };

  struct desc_table_info idti = { 16 * INT_VECTOR_NUMBER - 1, IDT_ADDR };
  __asm__("lidt %0" : : "m"(idti));
}

void init_desc_tables(void) {
  create_gdt();
  for (volatile int i = 0; i < 1; i++); // workaround for clang with -O2/-O3
  create_idt();
}

void *get_isr(int vector) {
  struct idt_desc *idt = (struct idt_desc*)IDT_ADDR + vector;
  return (void*)(idt->handler0 + ((uint64_t)idt->handler1 << 16) +
                 ((uint64_t)idt->handler2 << 32));
}

void set_isr(int vector, void *isr) {
  struct idt_desc *idt = (struct idt_desc*)IDT_ADDR + vector;
  uint64_t isri = (uint64_t)isr;
  idt->present = isri != 0;
  idt->handler0 = (uint16_t)isri;
  idt->handler1 = (uint16_t)(isri >> 16);
  idt->handler2 = (uint32_t)(isri >> 32);
}
