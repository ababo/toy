.section .boot
.global __start
__start:
        mov w0, 33
        mov x1, 0x9000000
        str w0, [x1,0]
        b .
