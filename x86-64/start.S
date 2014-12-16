/*
 * Boot sequences before switching to long mode.
 */

#include "config.h"
#include "cpu.h"

        .global start32
        .global halt

#define PIC1_DATA_PORT 0x21
#define PIC2_DATA_PORT 0xA1

start32:
        .code32
        movl %ebx, multiboot_info
        movb $0xFF, %al
        outb %al, $PIC1_DATA_PORT
        outb %al, $PIC2_DATA_PORT
        movl %cr4, %edx
        orl $(CR4_PAE | CR4_OSFXSR), %edx
        movl %edx, %cr4
        movl $pml4, %eax
        movl %eax, %cr3


        movl boot_stack, %esp
        addl $BOOT_STACK_SIZE, %esp
        call boot

halt:
        hlt
        jmp halt
