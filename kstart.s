.set DATA_SEGMENT, 0x0010
.set OSFXSR_MASK, 1 << 9

.section .text
.code64

kstart: movw $DATA_SEGMENT, %ax
        movw %ax, %ds
        movw %ax, %es
        movw %ax, %fs
        movw %ax, %gs

        movq %cr4, %rdx         /* enable SSE */
        orq $OSFXSR_MASK, %rdx
        movq %rdx, %cr4

        callq kmain
halt:   hlt
        jmp halt
