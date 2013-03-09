#include "config.h"
#include "multiboot.h"
#include "page_map.h"
#include "sys_table.h"
#include "util.h"

#define HEADER_FLAGS (MULTIBOOT_HEADER_MEMOTY_MAP)

SECTION(".mbh") const struct multiboot_header multiboot_header = {
  .magic = MULTIBOOT_HEADER_MAGIC, .flags = HEADER_FLAGS,
  .checksum = -(MULTIBOOT_HEADER_MAGIC + HEADER_FLAGS)
};

static inline void set_page_desc(struct sys_page_desc *desc, bool present,
                                 bool page, uint64_t address) {
  if (present) {
    desc->present = desc->write = true, desc->ps_pat = !!page;
    desc->address0 = (uint32_t)(address >> 12);
    desc->address1 = (uint32_t)(address >> 32);
  }
  else
    *desc = (struct sys_page_desc) { .present = false };
}

static void create_page_map(void *map, uint64_t addr_space_size,
                            uint64_t mapped_addr, uint64_t mapped_size) {
  struct sys_page_desc *pml4 = map, *pdpt, *pd, *next_table = pml4 + 512;
  uint64_t total_pages = SIZE_ELEMENTS(addr_space_size, PAGE_SIZE);
  uint64_t last_page = (mapped_addr + mapped_size - 1) >> 21;
  uint64_t first_page = mapped_addr >> 21, page = 0;

  for (int pml4i = 0; ; pml4i++) {
    set_page_desc(&pml4[pml4i], true, false, (size_t)next_table);
    pdpt = next_table, next_table += 512;

    for (int pdpti = 0; pdpti < 512; pdpti++) {
      set_page_desc(&pdpt[pdpti], true, false, (size_t)next_table);
      pd = next_table, next_table += 512;

      for (int pdi = 0; pdi < 512; pdi++) {
        bool present = page >= first_page && page <= last_page;
        set_page_desc(&pd[pdi], present, true, page++ << 21);
      }

      if (page >= total_pages)
        return;
    }
  }
}

static void create_gdt(void *gdt, struct sys_table_info *gdti) {
  struct sys_gdt_desc *desc = gdt;
  desc[1] = (struct sys_gdt_desc) {
    .type = SYS_SEGMENT_CODE, .nonsys = true, .present = true, .bits64 = true
  };
  desc[2] = (struct sys_gdt_desc) {
    .type = SYS_SEGMENT_DATA, .nonsys = true, .present = true, .bits32 = true
  };
  gdti->base = (size_t)gdt, gdti->limit = (8 * 3) - 1;
}

#define CR0_PG (1 << 31)
#define CR4_OSFXSR (1 << 9)
#define CR4_PAE (1 << 5)
#define MSR_EFER 0xC0000080
#define MSR_EFER_LME (1 << 8)
#define SEGMENT_CODE (1 << 3)

uint32_t multiboot_info = 0;
ALIGNED(16) uint8_t bsp_boot_stack[CONFIG_BSP_BOOT_STACK_SIZE] = { 0 };

ASM(".text\n.global bstart32\n"
    "bstart32: movl $(bsp_boot_stack + "
      STR_EXPAND(CONFIG_BSP_BOOT_STACK_SIZE) "), %esp\n"
    "movl %ebx, multiboot_info\n"
    "movl %cr4, %edx\n" // enable SSE
    "orl $" STR_EXPAND(CR4_OSFXSR) ", %edx\n"
    "movl %edx, %cr4\n"
    "call boot32\n"
    "halt: hlt\njmp halt");

ALIGNED(4096) uint8_t page_map[PAGE_MAP_SIZE] = { 0 };
ALIGNED(4) uint8_t gdt[(3 + 2 * CONFIG_CPUS_MAX) * 8] = { 0 };

void boot32(void) {
  extern int lds_kernel_size;
  struct sys_table_info gdti;
  create_page_map(page_map, CONFIG_ADDR_SPACE_SIZE, CONFIG_KERNEL_ADDR,
                  (size_t)&lds_kernel_size);
  create_gdt(gdt, &gdti);
  ASMV("movl %%cr4, %%eax\norl %0, %%eax\nmovl %%eax, %%cr4"
       : : "i"(CR4_PAE) : "eax");
  ASMV("mov %0, %%cr3" : : "d"((size_t)page_map));
  wrmsr(MSR_EFER, rdmsr(MSR_EFER) | MSR_EFER_LME);
  ASMV("movl %%cr0, %%eax\norl %0, %%eax\nmovl %%eax, %%cr0"
       : : "i"(CR0_PG) : "eax");
  ASMV("lgdt %0" : : "m"(gdti));
  ASMV("ljmp %0, $kstart" : : "i"(SEGMENT_CODE));
}
