OUTPUT_FORMAT(elf64-x86-64)
ENTRY(boot)
SECTIONS {
  . = 0x10;
  .text : { ../bin/boot.o(.text) }
  .text : { *(.text) }
  .data : { *(.data) }
  .bss : { *(.bss) }
  .rodata : { *(.rodata*) }
  /DISCARD/ : { *(*) }
}