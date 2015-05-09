	.set PIC1_DATA_PORT, 0x21
	.set PIC2_DATA_PORT, 0xA1

	.set CR0_PG, 1 << 31

	.set CR4_PAE, 1 << 5
	.set CR4_OSFXSR, 1 << 9

	.set PTE_PRESENT, 1 << 0
	.set PTE_WRITE, 1 << 1
	.set PTE_PS, 1 << 7

	.set MSR_EFER, 0xC0000080
	.set MSR_EFER_LME, 1 << 8

	.set GDT_TYPE_DATA, 0x2 << 40
	.set GDT_TYPE_CODE, 0x9 << 40
	.set GDT_NONSYS, 1 << 44
	.set GDT_PRESENT, 1 << 47
	.set GDT_BITS32, 1 << 53
	.set GDT_BITS64, 1 << 54

	.set SEGMENT_CODE, 0x8
	.set SEGMENT_DATA, 0x10

	.set BOOT_STACK_SIZE, 8192

	.global __boot_stack
	.global __pml4
	.global __pdp0
	.global __pd0
	.global __gdt
	.global __start32
	.global __halt

	.bss
	.align 4096
__pml4:
	.fill 512, 8
__pdp0:
	.fill 512, 8
__pd0:
	.fill 512, 8

	.align 16
__boot_stack:
	.fill BOOT_STACK_SIZE

	.data
	.align 4
__gdt:
	.quad 0
	.quad GDT_TYPE_CODE | GDT_NONSYS | GDT_PRESENT | GDT_BITS64
	.quad GDT_TYPE_DATA | GDT_NONSYS | GDT_PRESENT | GDT_BITS32
end_of_gdt:

gdti:
	.word end_of_gdt - __gdt - 1
	.quad __gdt

	.text
	.code32
__start32:
	/* preserve magic and multiboot_info */
	movl %eax, %edi
	movl %ebx, %esi

	/* disable IRQs */
	movb $0xFF, %al
	outb %al, $PIC1_DATA_PORT
	outb %al, $PIC2_DATA_PORT

	/* enable PAE and SSE */
	movl %cr4, %edx
	orl $(CR4_PAE | CR4_OSFXSR), %edx
	movl %edx, %cr4

	/* link page table entries and set page map */
	orl $(PTE_PRESENT | PTE_WRITE), __pml4
	orl $__pdp0, __pml4
	orl $(PTE_PRESENT | PTE_WRITE), __pdp0
	orl $__pd0, __pdp0
	orl $(PTE_PRESENT | PTE_WRITE | PTE_PS), __pd0
	movl $__pml4, %eax
	movl %eax, %cr3

	/* enable long mode */
	movl $MSR_EFER, %ecx
	rdmsr
	orl $MSR_EFER_LME, %eax
	wrmsr

	/* enable paging */
	movl %cr0, %eax
	orl $CR0_PG, %eax
	movl %eax, %cr0

	/* load GDT */
	lgdt gdti

	/* long jump to 64-bit code */
	ljmp $SEGMENT_CODE, $start64

	.code64
start64:
	/* set segments and stack */
	movw $SEGMENT_DATA, %ax
	movw %ax, %ds
	movw %ax, %ss
	movq $(__boot_stack + BOOT_STACK_SIZE), %rsp

	/* extend preserved magic and multiboot_info */
	shlq $32, %rdi
	shrq $32, %rdi
	shlq $32, %rsi
	shrq $32, %rsi

	/* call the Rust entry point */
	call __boot

__halt:
	/* halt CPU */
	hlt
	jmp __halt
