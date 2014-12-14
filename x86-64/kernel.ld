OUTPUT_FORMAT(elf64-x86-64)

ENTRY(__start32)

SECTIONS {
  . = 0x100000;
  .mbh : { *(.mbh) }
  .text : { *(.text) }
  .rodata : { *(.rodata*) }
  .data : { *(.data) }
  .bss : { *(.bss); __kernel_size = . - 0x100000; }
  /DISCARD/ : { *(*) }
}
