.set DATA_SEGMENT, 0x0010

.section .text
.code64

kstart: movw $DATA_SEGMENT, %ax
        movw %ax, %ds
        movw %ax, %es
        movw %ax, %fs
        movw %ax, %gs
        call kmain
        hlt
