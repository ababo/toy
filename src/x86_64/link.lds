SECTIONS {
  . = 0;
  .text : { KEEP(*(.boot)) *(.text*) }
  .rodata : { *(.rodata*) }
  .data : { *(.data) }
  .bss : { *(.bss) }
  /DISCARD/ : { *(*) }
}
