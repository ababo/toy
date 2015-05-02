
	.set PIC1_DATA_PORT, 0x21
	.set PIC2_DATA_PORT, 0xA1

	.set CR4_PAE, 1 << 5
	.set CR4_OSFXSR, 1 << 9

	.set MSR_EFER, 0xC0000080
	.set MSR_EFER_LME, 1 << 8

	.set CR0_PG, 1 << 31

	.set SEGMENT_CODE, 0x8
	.set SEGMENT_DATA, 0x10

	.set BOOT_STACK_SIZE, 0x2000

        .global start32
        .global halt

start32:
        .code32
        movl %ebx, multiboot_info

        movb $0xFF, %al                         /* disable IRQs */
        outb %al, $PIC1_DATA_PORT
        outb %al, $PIC2_DATA_PORT

        movl %cr4, %edx                         /* enable PAE and SSE */
        orl $(CR4_PAE | CR4_OSFXSR), %edx
        movl %edx, %cr4

        orl $__pdpe, pml4                       /* link page table entries */
        orl $__pde, __pdpe                      /* and set page map */
        movl $pml4, %eax
        movl %eax, %cr3

        movl $MSR_EFER, %ecx                    /* enable long mode */
        rdmsr
        orl $MSR_EFER_LME, %eax
        wrmsr

        movl %cr0, %eax                         /* enable paging */
        orl $CR0_PG, %eax
        movl %eax, %cr0

        lgdt __gdti                             /* load GDT */

        ljmp $SEGMENT_CODE, $start64            /* long jump to 64-bit code */

start64:
        .code64
        movw $SEGMENT_DATA, %ax                 /* set segments and stack */
        movw %ax, %ds
        movw %ax, %ss
        movq $(boot_stack + BOOT_STACK_SIZE), %rsp

        call boot                               /* call the first C-function */

halt:
        hlt
        jmp halt
