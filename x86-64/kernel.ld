/*
 * Linker script to build kernel.
 */

OUTPUT_FORMAT(elf64-x86-64)

ENTRY(start32)

SECTIONS {
  . = 0x100000;
  .mbh : { *(.mbh) }
  .text : { *(.text) }
  .rodata : { *(.rodata*) }
  .data : { *(.data) }
  .bss : { *(.bss); kernel_size = . - 0x100000; }
  /DISCARD/ : { *(*) }
}
