.set DATA_SEGMENT, 0x0010

.section .text
.code64

kstart: movw $DATA_SEGMENT, %ax
        movw %ax, %ds
        movw %ax, %es
        movw %ax, %fs
        movw %ax, %gs

        movl $0xB8000, %edi
        movq $500, %rcx
        movq $0x1F201F201F201F20, %rax
        rep stosq
        jmp .

        call kmain
