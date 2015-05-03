OUTPUT_FORMAT(elf32-i386)

ENTRY(__start32)

SECTIONS {
  . = 1M;
  .multiboot : { *(.multiboot) }
  .text : { *(.text) }
  .rodata : { *(.rodata*) }
  .data : { *(.data) }
  .bss : {
    *(.bss);
    *(COMMON);
    kernel_size = . - 1M;
  }
  /DISCARD/ : { *(*) }
}