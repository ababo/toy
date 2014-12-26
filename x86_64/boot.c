/**
 * Boot sequence after switching to long mode.
 **/

#include "config.h"
#include "klog.h"
#include "std.h"
#include "x86_64/cpu.h"
#include "x86_64/multiboot.h"

#define HEADER_FLAGS (MULTIBOOT_MEMORY_INFO)

const struct multiboot_header
multiboot_header __attribute__((section(".multiboot"))) = {
  .magic = MULTIBOOT_HEADER_MAGIC,
  .flags = HEADER_FLAGS,
  .checksum = (multiboot_uint32_t)-(MULTIBOOT_HEADER_MAGIC + HEADER_FLAGS)
};

uint32_t multiboot_info = 0;

/* PMP4 table and 2 page table entries (of PDP and PD) to
   map 1:1 the first 2M-page before switching to long mode */

struct page_table_entry
pml4[PAGE_TABLE_ENTRIES] __attribute__((aligned(4096))) = {
  { .present = 1, .write = 1 }
};

struct page_table_entry __pdpe __attribute__((aligned(4096))) = {
  .present = 1, .write = 1
};

struct page_table_entry __pde __attribute__((aligned(4096))) = {
  .present = 1, .write = 1, .ps_pat = 1
};

struct gdt gdt __attribute__((aligned(4))) = {
  .code = { .type = GDT_CODE, .nonsys = 1, .present = 1, .bits64 = 1 },
  .data = { .type = GDT_DATA, .nonsys = 1, .present = 1, .bits32 = 1 }
};

struct desc_table_info __gdti = { sizeof(gdt) - 1, (uint64_t)&gdt };

uint8_t boot_stack[BOOT_STACK_SIZE] __attribute__((aligned(16)));

static void putc(void *arg, uint32_t chr) {
  (void)arg;
  __asm__ volatile ("movw 0x400, %%dx; movb %0, %%al; outb %%al, %%dx" : : "r"((char)chr));
}

void boot() {
  klog_init(putc, NULL, INFO);
  klog(INFO, "Hello worl%c!", 'd');
}
