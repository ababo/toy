#include "config.h"
#include "multiboot.h"
#include "page_map.h"
#include "sys_table.h"
#include "util.h"

#define HEADER_FLAGS (MULTIBOOT_HEADER_MEMOTY_MAP)

USED static const struct multiboot_header header = {
  .magic = MULTIBOOT_HEADER_MAGIC, .flags = HEADER_FLAGS,
  .checksum = -(MULTIBOOT_HEADER_MAGIC + HEADER_FLAGS)
};

uint32_t multiboot_info = 0;
uint8_t bsp_boot_stack[CONFIG_BSP_BOOT_STACK_SIZE] = { };

ASM(".global bstart32\n"
    "bstart32: movl $(bsp_boot_stack + "
      STR_EXPAND(CONFIG_BSP_BOOT_STACK_SIZE)
      "), %esp\n"
    "movl %ebx, multiboot_info\n"
    "call boot32\n"
    "halt: hlt\njmp halt");

extern const uint32_t kernel_size;
struct sys_page_desc page_map[PAGE_MAP_SIZE / 8] = { };

static void set_paging(void) {

}

uint64_t gdt[3 + 2 * CONFIG_CPUS_MAX] = { };

static void set_gdt(void) {
  *(struct sys_gdt_desc*)&gdt[1] = (struct sys_gdt_desc) {
    .type = SYS_SEGMENT_CODE, .nonsys = true, .present = true, .bits64 = true
  };
  *(struct sys_gdt_desc*)&gdt[2] = (struct sys_gdt_desc) {
    .type = SYS_SEGMENT_DATA, .nonsys = true, .present = true, .bits32 = true
  };

  struct sys_table_info gdti; // clang crushes with static initializer
  gdti.base = (uint64_t)(size_t)&gdt;
  gdti.limit = (8 * 3) - 1;
  ASMV("lgdt %0" : : "m"(gdti));
}

#define CR0_PG (1 << 31)
#define CR4_PAE (1 << 5)
#define MSR_EFER 0xC0000080
#define MSR_EFER_LME (1 << 8)

#define SEGMENT_CODE (1 << 3)

void boot32(void) {
  ASMV("movl %0, %%cr4" : : "a"(CR4_PAE));
  set_paging();
  wrmsr(MSR_EFER, rdmsr(MSR_EFER) | MSR_EFER_LME);
  ASMV("movl %%cr0, %%eax\norl %0, %%eax\nmovl %%eax, %%cr0"
       : : "i"(CR0_PG) : "eax");
  set_gdt();
  ASMV("ljmp $" STR_EXPAND(SEGMENT_CODE) ", $kstart");

  // I'm a sanity test, remove when done
  *(char*)0xB8000 = 'H', *(char*)0xB8002 = 'e', *(char*)0xB8004 = 'l';
  *(char*)0xB8006 = 'l', *(char*)0xB8008 = 'o', *(char*)0xB800A = '!';
}
