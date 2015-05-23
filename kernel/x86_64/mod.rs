mod multiboot;

const HEADER_FLAGS: u32 = multiboot::HEADER_MEMORY_INFO;

#[link_section=".multiboot_header"]
pub static MULTIBOOT_HEADER: multiboot::Header = multiboot::Header{
    magic: multiboot::HEADER_MAGIC,
    flags: HEADER_FLAGS,
    checksum: (-((multiboot::HEADER_MAGIC + HEADER_FLAGS) as i32) as u32),
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

#[no_mangle]
pub extern fn __boot(magic: u32, _info: &multiboot::Info) {
    if magic != multiboot::BOOTLOADER_MAGIC {
        return
    }

    unsafe {
        asm!("movw 0x400, %dx; movb $$('!'), %al; outb %al, %dx");
    }
}