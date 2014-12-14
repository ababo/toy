/*
 * Boot sequence after switching to long mode.
 */

#include "common.h"
#include "config.h"
#include "multiboot.h"

#define HEADER_FLAGS (MULTIBOOT_MEMORY_INFO)

const struct multiboot_header __multiboot_header
__attribute__((section(".mbh"))) = {
  .magic = MULTIBOOT_HEADER_MAGIC, .flags = HEADER_FLAGS,
  .checksum = -(MULTIBOOT_HEADER_MAGIC + HEADER_FLAGS)
};

uint8_t __boot_stack[BOOT_STACK_SIZE] __attribute__((aligned(16))) = { 0 };

void __boot() {

}
