/*
 * Boot sequences before switching to long mode.
 */
        .global __start32
        .global __halt

#include "config.h"
#include "cpu.h"

#define PIC1_DATA_PORT 0x21
#define PIC2_DATA_PORT 0xA1

__start32:
        .code32
        movl %ebx, __multiboot_info
        movb $0xFF, %al
        outb %al, $PIC1_DATA_PORT
        outb %al, $PIC2_DATA_PORT
        movl %cr4, %edx
        orl $(CR4_PAE | CR4_OSFXSR), %edx
        movl %edx, %cr4
        movl $__pml4, %eax
        movl %eax, %cr3


        movl __boot_stack, %esp
        addl $BOOT_STACK_SIZE, %esp
        call __boot

__halt:
        hlt
        jmp __halt
