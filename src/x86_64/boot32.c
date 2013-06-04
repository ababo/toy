#include "../common.h"
#include "../config.h"
#include "cpu.h"
#include "multiboot.h"
#include "page_map.h"

#define HEADER_FLAGS (MULTIBOOT_HEADER_MEMOTY_MAP)

SECTION(".mbh") const struct multiboot_header __multiboot_header = {
  .magic = MULTIBOOT_HEADER_MAGIC, .flags = HEADER_FLAGS,
  .checksum = -(MULTIBOOT_HEADER_MAGIC + HEADER_FLAGS)
};

static inline void set_page_desc(struct page_desc *desc, bool present,
                                 bool page, uint64_t address) {
  if (present) {
    desc->present = desc->write = true, desc->ps_pat = !!page;
    desc->address0 = (uint32_t)(address >> PAGE_DESC_ADDR0_BS);
    desc->address1 = (uint32_t)(address >> PAGE_DESC_ADDR1_BS);
  }
  else
    *desc = (struct page_desc) { .present = false };
}

static void create_page_map(void *map, uint64_t addr_space_size,
                            uint64_t mapped_addr, uint64_t mapped_size) {
  struct page_desc *pml4 = map, *pdpt, *pd;
  struct page_desc *next_table = pml4 + PAGE_TABLE_DESCS;
  uint64_t total_pages = SIZE_ELEMENTS(addr_space_size, PAGE_SIZE);
  uint64_t last_page = (mapped_addr + mapped_size - 1) / PAGE_SIZE;
  uint64_t first_page = mapped_addr / PAGE_SIZE, page = 0;

  for (int pml4i = 0; ; pml4i++) {
    set_page_desc(&pml4[pml4i], true, false, (size_t)next_table);
    pdpt = next_table, next_table += PAGE_TABLE_DESCS;

    for (int pdpti = 0; pdpti < PAGE_TABLE_DESCS; pdpti++) {
      set_page_desc(&pdpt[pdpti], true, false, (size_t)next_table);
      pd = next_table, next_table += PAGE_TABLE_DESCS;

      for (int pdi = 0; pdi < PAGE_TABLE_DESCS; pdi++) {
        bool present = page >= first_page && page <= last_page;
        set_page_desc(&pd[pdi], present, true, page++ * PAGE_SIZE);
      }

      if (page >= total_pages)
        return;
    }
  }
}

static void create_gdt(void *gdt, struct cpu_table_info *gdti) {
  struct gdt_desc *desc = gdt;
  desc[1] = (struct gdt_desc) {
    .type = GDT_SEGMENT_CODE, .nonsys = true, .present = true, .bits64 = true
  };
  desc[2] = (struct gdt_desc) {
    .type = GDT_SEGMENT_DATA, .nonsys = true, .present = true, .bits32 = true
  };
  gdti->base = (size_t)gdt, gdti->limit = (GDT_DESC_SIZE * 3) - 1;
}

uint32_t __multiboot_info = 0;
ALIGNED(16) uint8_t __bsp_boot_stack[CONFIG_BSP_BOOT_STACK_SIZE] = { };

ASM(".text; .global __bstart32; .global __halt;"
    "__bstart32: movl $(__bsp_boot_stack + "
      STR_EXPAND(CONFIG_BSP_BOOT_STACK_SIZE) "), %esp;"
    "movl %ebx, __multiboot_info;"
    "movb $0xFF, %al; outb %al, $0xA1; outb %al, $0x21;" // disable IRQs
    "movl %cr4, %edx;" // enable SSE
    "orl $" STR_EXPAND(CR4_OSFXSR) ", %edx;"
    "movl %edx, %cr4;"
    "call boot32;"
    "__halt: hlt; jmp __halt");

ALIGNED(4096) uint8_t __page_map[PAGE_MAP_SIZE] = { };
ALIGNED(4) uint8_t __gdt[(3 + 2 * CONFIG_CPUS_MAX) * GDT_DESC_SIZE] = { };

void boot32(void) {
  extern int __lds_kernel_size;
  struct cpu_table_info gdti;
  create_page_map(__page_map, CONFIG_ADDR_SPACE_SIZE, CONFIG_KERNEL_ADDR,
                  (size_t)&__lds_kernel_size);
  create_gdt(__gdt, &gdti);
  ASMV("movl %%cr4, %%eax; orl %0, %%eax; movl %%eax, %%cr4"
       : : "i"(CR4_PAE) : "eax");
  ASMV("mov %0, %%cr3" : : "d"((size_t)__page_map));
  wrmsr(MSR_EFER, rdmsr(MSR_EFER) | MSR_EFER_LME);
  ASMV("movl %%cr0, %%eax; orl %0, %%eax; movl %%eax, %%cr0"
       : : "i"(CR0_PG) : "eax");
  ASMV("lgdt %0" : : "m"(gdti));
  ASMV("ljmp %0, $__kstart" : : "i"(SEGMENT_CODE));
}
