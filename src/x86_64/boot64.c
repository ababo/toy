#include "../common.h"
#include "../config.h"
#include "../cpu_info.h"
#include "acpi.h"
#include "cpu.h"
#include "vga.h"

ASM(".text\n.global kstart\n"
    "kstart: movw $" STR_EXPAND(SEGMENT_DATA) ", %ax\n"
    "movw %ax, %ds\nmovw %ax, %ss\n"
    "movq $(bsp_boot_stack + "
      STR_EXPAND(CONFIG_BSP_BOOT_STACK_SIZE) "), %rsp\n"
    "call boot64\n"
    "jmp halt");

void kmain(void);

void boot64(void) {
  init_vga();
  init_acpi();
  init_cpu_info();

  kmain();
}
