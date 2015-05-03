mod cpu;
mod multiboot;

#[link_section=".multiboot_header"]
pub static MULTIBOOT_HEADER: multiboot::Header = multiboot::Header{
    magic: multiboot::HEADER_MAGIC,
    flags: multiboot::HEADER_MEMORY_INFO,
    checksum: 0,
    header_addr: 0,
    load_addr: 0,
    load_end_addr: 0,
    bss_end_addr: 0,
    entry_addr: 0,
    mode_type: 0,
    width: 0,
    height: 0,
    depth: 0
};

const PIC1_DATA_PORT: u16 = 0x21;
const PIC2_DATA_PORT: u16 = 0xA1;

pub unsafe fn __asm_wrapper() {
    asm!("
        .code32
        .global __start32

__start32:
        movl %eax, %esi                     /* save magic and multiboot_info */
        movl %ebx, %edi

        movb $$0xFF, %al                    /* disable IRQs */
        outb %al, $0
        outb %al, $1

        movl %cr4, %edx                     /* enable PAE and SSE */
        orl $2, %edx
        movl %edx, %cr4

        "
        :
        : "i"(PIC1_DATA_PORT)                               // 0
        , "i"(PIC2_DATA_PORT)                               // 1
        , "i"(cpu::CR4_PAE_BIT | cpu::CR4_OSFXSR_BIT)       // 2
        );
}

#[no_mangle]
pub extern fn __boot(_magic: u32, _info: &multiboot::Info) {
    unsafe {
        let ptr = 0xB8000 as *mut u8;
        *ptr = '!' as u8;
    }
}
