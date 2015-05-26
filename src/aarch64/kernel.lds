OUTPUT_FORMAT(elf64-littleaarch64)

ENTRY(__start)

SECTIONS {
	. = 0;
	.text : {
		__kernel_begin = .;
		*(.text*);
	}
	.rodata : {
		*(.rodata*);
	}
	.data : {
		*(.data*);
	}
	.bss : {
		*(.bss*);
		*(COMMON*);
		__kernel_end = .;
	}
/* linker error: discarded output section: `.got.plt'
	/DISCARD/ : {
		*(*);
	}
*/
}