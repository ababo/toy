ENTRY(__start32)

SECTIONS {
  . = 0x100000;
  .mbh : { *(.mbh) }
  .text : { *(.text*) }
  .rodata : { *(.rodata*) }
  .data : { *(.data) }
  .bss : { *(.bss) }
  /DISCARD/ : { *(*) }
}

