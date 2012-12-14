OUTPUT_FORMAT(binary)
SECTIONS {
  .text 0x7E00 : AT ( 0x7E00 ){ kstart.o(.text) } = 0x9090
  .text : { *(.text) } = 0x9090
  .data : { *(.data) } = 0x0000
  /DISCARD/ : { *(*) }
}