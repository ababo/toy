.set LOAD_ADDRESS, 0x7C00
.set MAGIC_WORD, 0xAA55
.set SECTOR_SIZE, 512
.set SECTORS_PER_TRACK, 18
.set NUMBER_OF_HEADS, 2
.set DRIVE_NUMBER, 0
.set KERNEL_ADDRESS, LOAD_ADDRESS + SECTOR_SIZE
.set KERNEL_LIMIT_ADDRESS, 0x9F600

.section .text
.code16
.global _start

_start: xorw %ax, %ax
        movw %ax, %es
        movw %ax, %ds
        movw %ax, %ss
        movw $LOAD_ADDRESS, %sp

        movw $1, %ax
        movw $(LOAD_ADDRESS + SECTOR_SIZE), %bx
        movw $((KERNEL_LIMIT_ADDRESS - KERNEL_ADDRESS) / SECTOR_SIZE), %cx
        call load

        movw $msg, %si
        call puts
        movw $(LOAD_ADDRESS + SECTOR_SIZE), %si
        call puts
        movw $(LOAD_ADDRESS + 2 * SECTOR_SIZE), %si
        call puts
        hlt

        /* ds:si - string address */
puts:   lodsb
        cmpb  $0x0, %al
        je puts1
        movw  $0x07, %bx
        movb  $0x0E, %ah
        int   $0x10
        jmp puts
puts1:  ret

        /* ax - index of first sector
           es:bx - destination address
           cx - number of sectors */
load:   pusha
        movb $SECTORS_PER_TRACK, %dl
        divb %dl                        /* T = N / SECTORS_PER_TRACK */
        movb %ah, %cl
        incb %cl                        /* S = N mod SECTORS_PER_TRACK + 1 */
        xorb %ah, %ah
        movb $NUMBER_OF_HEADS, %dl
        divb %dl
        movb %al, %ch                   /* C = T / NUMBER_OF_HEADS */
        movb %ah, %dh                   /* H = T mod NUMBER_OF_HEADS */
        movb $DRIVE_NUMBER, %dl
        movw $0x0201, %ax
        int $0x13
        movw %es, %ax
        addw $(SECTOR_SIZE >> 4), %ax
        movw %ax, %es
        popa
        incw %ax
        loop load
        ret

msg:    .asciz "Hello World!\r\n"

        .org SECTOR_SIZE - 2
        .word MAGIC_WORD
