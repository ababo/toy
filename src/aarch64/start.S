#include "aarch64/config.h"

        .global __boot_stack

        .bss
__boot_stack:
        .space __BOOT_STACK_SIZE
stack_end:

        .section .boot
        adr x1, stack_end
        mov sp, x1
        bl __boot
halt:
        wfi
        b halt
