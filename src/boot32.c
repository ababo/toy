#include "config.h"
#include "multiboot.h"
#include "util.h"

#define HEADER_FLAGS (MULTIBOOT_HEADER_MEMOTY_MAP)

USED static const struct multiboot_header header = {
  .magic = MULTIBOOT_HEADER_MAGIC, .flags = HEADER_FLAGS,
  .checksum = -(MULTIBOOT_HEADER_MAGIC + HEADER_FLAGS)
};

uint32_t multiboot_info = 0;
uint8_t bsp_boot_stack[CONFIG_BSP_BOOT_STACK_SIZE] = { };

ASM(".global bstart32\n"
    "bstart32: movl %ebx, multiboot_info\n"
    "movl $(bsp_boot_stack + " STR_EX(CONFIG_BSP_BOOT_STACK_SIZE) "), %esp\n"
    "call boot32\n"
    "halt: hlt\njmp halt");

void boot32(void) {
    *(char*)0xB8000 = 50;
    *(char*)0xB8002 = 49;
    *(char*)0xB8004 = 48;
}
