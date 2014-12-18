/*
 * Linker script to build kernel.
 */

OUTPUT_FORMAT(elf32-i386)

ENTRY(start32)

SECTIONS {
  . = 1M;
  .mbh : { *(.mbh) }
  .text : { *(.text) }
  .rodata : { *(.rodata*) }
  .data : { *(.data) }
  .bss : { *(.bss); kernel_size = . - 1M; }
  /DISCARD/ : { *(*) }
}
