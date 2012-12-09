.set LOAD_ADDRESS, 0x7C00
.set MAGIC_WORD, 0xAA55
.set BLOCK_SIZE, 512

.section .text
.code16
.global _start

_start: xorw %ax, %ax
        movw %ax, %es
        movw %ax, %ds
        movw %ax, %ss
        movw $LOAD_ADDRESS, %sp

        movw $msg, %si
        call puts
        hlt

puts:   lodsb
        cmpb  $0x0, %al
        je puts1
        movw  $0x7, %bx
        movb  $0xe, %ah
        int   $0x10
        jmp puts
puts1:  ret

msg:    .asciz "Hello World!\r\n"

        .org BLOCK_SIZE - 2
        .word MAGIC_WORD
