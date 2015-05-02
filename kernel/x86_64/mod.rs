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

#[no_mangle]
pub extern fn boot(_magic: u32, _info: &multiboot::Info) {
    unsafe {
        let ptr = 0xB8000 as *mut u8;
        *ptr = '!' as u8;
    }
}
