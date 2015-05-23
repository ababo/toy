#[no_mangle]
pub extern fn __boot() {
	unsafe {
        let ptr = 0x9000000 as *mut u8;
        *ptr = '!' as u8;
    }
}