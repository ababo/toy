#include "acpi.h"
#include "config.h"
#include "cpu_info.h"
#include "interrupt.h"
#include "mem_mgr.h"
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

ISR_DEFINE(test, 0) {
  printf("#DE: jumping to next instruction.\n");
  stack_frame->rip += 2;
}

void kmain(void) {
  init_vga();
  init_acpi();
  init_cpu_info();
  init_mem_mgr();
  init_interrupts(true);
  printf("Trying to initiate #DE.\n");
  set_isr(INT_VECTOR_DE, test_isr_getter());
  ASMV("div %P0" : : "a"(0));
  printf("Returned from #DE.\n");
}
