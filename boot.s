.set LOAD_ADDRESS, 0x7C00
.set MAGIC_WORD, 0xAA55
.set SECTOR_SIZE, 512
.set SECTORS_PER_TRACK, 18
.set NUMBER_OF_HEADS, 2
.set DRIVE_NUMBER, 0
.set KERNEL_ADDRESS, LOAD_ADDRESS + SECTOR_SIZE
.set KERNEL_LIMIT_ADDRESS, 0x9F600
.set PML4_ADDRESS, 0x1000
.set PAGE_PRESENT_BIT, 1 << 0
.set PAGE_WRITE_BIT, 1 << 1
.set PAE_BIT, 1 << 5
.set PGE_BIT, 1 << 7
.set EFER_MSR, 0xC0000080
.set LME_BIT, 1 << 8
.set PAGING_BIT, 1 << 0
.set PROTECTION_BIT, 1 << 31
.set CODE_SEGMENT_BIT, 1 << 3
.set CODE_DATA_SEGMENT_BIT, 1 << 4
.set DEFINED_SEGMENT_BIT, 1 << 7
.set L_BIT, 1 << 5
.set CODE_SEGMENT, 0x0008

.section .text
.code16
.global _start

_start: xorw %ax, %ax
        movw %ax, %es
        movw %ax, %ds
        movw %ax, %ss
        movw $LOAD_ADDRESS, %sp

        movw $msg, %si
        call puts                       /* display message */

        movw $1, %ax
        movw $(LOAD_ADDRESS + SECTOR_SIZE), %bx
        movw $((KERNEL_LIMIT_ADDRESS - KERNEL_ADDRESS) / SECTOR_SIZE), %cx
        call load                       /* load kernel */

        xorw %ax, %ax
        movw %ax, %es
        movl $PML4_ADDRESS, %edi

        pushw %di                       /* fill the buffer with zeros */
        mov $0x1000, %ecx
        xor %eax, %eax
        cld
        rep stosl
        pop %di

        movw %es, %ax                   /* prepare PML4 */
        shrl $4, %eax
        addl %edi, %eax
        addl $0x1000, %eax
        orl $(PAGE_PRESENT_BIT | PAGE_WRITE_BIT), %eax
        movl %eax, %es:(%di)

        addl $0x1000, %eax              /* prepare PDPT */
        movl %eax, %es:0x1000(%di)

        addl $0x1000, %eax
        movl %eax, %es:0x2000(%di)

        movw %es, %ax                   /* prepare PD */
        addw $0x300, %ax
        pushw %es
        movw %ax, %es
        movl $(PAGE_PRESENT_BIT | PAGE_WRITE_BIT), %eax

nextp:  movl %eax, %es:(%di)            /* prepare PT */
        addl $0x1000, %eax
        movl %eax, %es:8(%di)
        addl $0x1000, %eax
        movw %es, %bx
        incw %bx
        movw %bx, %es 
        cmpl $0x200000, %eax
        jb nextp
        popw %es

        movb $0xFF, %al                 /* disable IRQs */
        outb %al, $0xA1
        outb %al, $0x21

        lidt idti

        movl $(PAE_BIT | PGE_BIT), %eax
        movl %eax, %cr4

        movl %edi, %edx                 /* load address of PML4 into cr3 */
        movl %edx, %cr3

        movl $EFER_MSR, %ecx
        rdmsr
        orl $LME_BIT, %eax
        wrmsr

        movl %cr0, %ebx
        orl $(PAGING_BIT | PROTECTION_BIT), %ebx
        movl %ebx, %cr0

        lgdt gdti

        ljmp $CODE_SEGMENT, $KERNEL_ADDRESS

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

        .align 4
idti:   .word 0
        .long 0

gdt:    .quad 0                         /* null segment */
        .long 0                         /* code segment */
        .byte 0
        .byte CODE_SEGMENT_BIT | CODE_DATA_SEGMENT_BIT | DEFINED_SEGMENT_BIT
        .word L_BIT
        .long 0                         /* data segment */
        .byte 0
        .byte CODE_DATA_SEGMENT_BIT | DEFINED_SEGMENT_BIT
        .word 0
        .align 4
        .word 0
gdti:   .word . - gdt - 1
        .long gdt

msg:    .asciz "Loading kernel...\r\n"

        .org SECTOR_SIZE - 2
        .word MAGIC_WORD
