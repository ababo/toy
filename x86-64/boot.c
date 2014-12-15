//
// Boot sequence after switching to long mode.
//

#include "common.h"
#include "config.h"
#include "cpu.h"
#include "multiboot.h"

#define HEADER_FLAGS (MULTIBOOT_MEMORY_INFO)

const struct multiboot_header
multiboot_header __attribute__((section(".mbh"))) = {
      .magic = MULTIBOOT_HEADER_MAGIC, .flags = HEADER_FLAGS,
  .checksum = -(MULTIBOOT_HEADER_MAGIC + HEADER_FLAGS)
};

uint32_t multiboot_info = 0;

// additional element is used in start.S to
// switch to long mode - will map first 1G 1:1
struct page_entry
pml4[PAGE_TABLE_ENTRIES + 1] __attribute__((aligned(4096))) = {
  { .present = 0 }
};

uint8_t boot_stack[BOOT_STACK_SIZE] __attribute__((aligned(16))) = { 0 };

void boot() {

}
