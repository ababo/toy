#![feature(asm, core, no_std, lang_items)]
#![no_std]

#[macro_use]
extern crate core;

pub mod libc;

#[macro_use]
pub mod log;

#[cfg(arch_x86_64)]
pub mod x86_64;
#[cfg(arch_aarch64)]
pub mod aarch64;

#[no_mangle]
#[lang = "begin_unwind"]
pub extern fn rust_begin_unwind(
	_: &core::fmt::Arguments, _: &'static str, _: usize) -> ! {
	loop {}
}