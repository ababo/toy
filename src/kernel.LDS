#include "config.h"

OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kstart32)
SECTIONS {
  . = CONFIG_KERNEL_ADDR;
  .text : { ../bin/boot32.o(.text) }
  .text : { *(.text) }
  .data : { *(.data) }
  .bss : { *(.bss) }
  .rodata : { *(.rodata*) }
  /DISCARD/ : { *(*) }
}