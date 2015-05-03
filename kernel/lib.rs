#![feature(asm, core, no_std)]
#![no_std]

extern crate core;

#[cfg(arch_x86_64)]
pub mod x86_64;

#[cfg(aarch_aarch64)]
pub mod aarch64;