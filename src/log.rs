use core::fmt;
use core::prelude::*;

fn write_nothing(_s: &str) {}

pub static mut WRITE: fn(&str) = write_nothing;
pub static mut LEVEL: Level = Level::Info;

pub const LEVEL_PREFIXES: [&'static str; 4] = [ "i ", "w ", "E ", "F" ];

#[derive(PartialEq, PartialOrd)]
pub enum Level {
	Info = 0,
	Warning = 1,
	Error = 2,
	Fatal = 3,
}

pub fn init(write: fn(&str), level: Level) {
	unsafe {
		WRITE = write;
		LEVEL = level;
	}
}

pub struct Writer;

impl fmt::Write for Writer {
	fn write_str(&mut self, s: &str) -> fmt::Result {
		unsafe { WRITE(s); }
		Ok(())
	}
}

macro_rules! log {
	($level:expr, $($arg:tt)*) => ({
		use core::fmt;
		unsafe {
			if $level >= log::LEVEL {
				let writer: &mut fmt::Write = &mut log::Writer;
				match writer.write_str(log::LEVEL_PREFIXES[$level as usize]) {
					_ => {}
				}
				match write!(writer, $($arg)*) {
            		_ => {}
        		}
        		match writer.write_str("\n") {
					_ => {}
				}
        	}
        }
	})
}

macro_rules! info {
	($($arg:tt)*) => (log!(log::Level::Info, $($arg)*))
}

macro_rules! warning {
	($($arg:tt)*) => (log!(log::Level::Warning, $($arg)*))
}

macro_rules! error {
	($($arg:tt)*) => (log!(log::Level::Error, $($arg)*))
}

macro_rules! fatal {
	($($arg:tt)*) => (log!(log::Level::Fatal, $($arg)*))
}