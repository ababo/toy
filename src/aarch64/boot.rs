use core::prelude::*;
use log;

fn write(s: &str) {
    for chr in s.chars() {
        unsafe {
            let ptr = 0x9000000 as *mut u8;
        	*ptr = chr as u8;
        }
    }
}

#[no_mangle]
pub extern fn __boot() {
	log::init(write, log::Level::Info);
	info!("Hello {}!", "World");
}