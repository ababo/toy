#include "acpi.h"
#include "config.h"
#include "util.h"
#include "vga.h"

#define SEGMENT_DATA (2 << 3)

ASM(".text\n.global kstart\n"
    "kstart: movw $" STR_EXPAND(SEGMENT_DATA) ", %ax\n"
    "movw %ax, %ds\n"
    "movq $(bsp_boot_stack + "
      STR_EXPAND(CONFIG_BSP_BOOT_STACK_SIZE) "), %rsp\n"
    "call kmain\n"
    "halt: hlt\njmp halt");

void kmain(void) {
  init_vga();
  init_acpi();
  init_cpu_info();
}
