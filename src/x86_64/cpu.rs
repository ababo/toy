pub unsafe fn send8(value: u8, port: u16) {
	asm!("outb $0, $1" : : "{al}"(value), "{dx}"(port));
}