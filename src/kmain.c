#include "config.h"
#include "util.h"

#define SEGMENT_DATA (2 << 3)
#define CR4_OSFXSR (1 << 9)

ASM(".global kstart\n"
    "kstart: movw $" STR_EXPAND(SEGMENT_DATA) ", %ax\n"
    "movw %ax, %ds\n"
    "movq $(bsp_boot_stack + "
      STR_EXPAND(CONFIG_BSP_BOOT_STACK_SIZE)
      "), %rsp\n"
    "movq %cr4, %rdx\n" // enable SSE
    "orq $" STR_EXPAND(CR4_OSFXSR) ", %rdx\n"
    "movq %rdx, %cr4\n"
    "call kmain\n"
    "halt: hlt\njmp halt");

void kmain(void) {
  // I'm a sanity test, remove when done
  *(char*)0xB8000 = 'H', *(char*)0xB8002 = 'e', *(char*)0xB8004 = 'l';
  *(char*)0xB8006 = 'l', *(char*)0xB8008 = 'o', *(char*)0xB800A = '!';
}
