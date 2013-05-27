#include "../config.h"

OUTPUT_FORMAT(elf64-x86-64)
ENTRY(__bstart32)
SECTIONS {
  . = CONFIG_KERNEL_ADDR;
  .mbh : { *(.mbh) }
  .text : { *(.text) }
  .rodata : { *(.rodata*) }
  .data : { *(.data) }
  .bss : {
    *(.bss)
    __lds_kernel_size = . - CONFIG_KERNEL_ADDR;
  }
  /DISCARD/ : { *(*) }
}
