OUTPUT_FORMAT(binary)
SECTIONS {
  .text 0x7E00 : { ../bin/kstart.o(.text) }
  .text : { *(.text) }
  .data : { *(.data) }
  .bss : { *(.bss) }
  .rodata : { *(.rodata*) }
  /DISCARD/ : { *(*) }
}
