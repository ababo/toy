OUTPUT_FORMAT(elf32-i386)

ENTRY(__start32)

SECTIONS {
	. = 1M;
	.multiboot_header : {
		__kernel_begin = .;
		*(.multiboot_header);
	}
	.text : {
		*(.text*);
	}
  	.rodata : {
  		*(.rodata*);
  	}
  	.data : {
  		*(.data);
  	}
	.bss : {
 		*(.bss);
		*(COMMON);
		__kernel_end = .;
  	}
	/DISCARD/ : {
		*(*);
	}
}