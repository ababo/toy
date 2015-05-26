#[no_mangle]
#[allow(non_upper_case_globals)]
pub static memcmp: isize = 0;

#[no_mangle]
pub unsafe extern fn memcpy(dst: *mut (), src: *const (),
							num: usize) -> *mut () {
	let mut curd = dst as *mut u8;
	let mut curs = src as *mut u8;
	let endd = curd.offset(num as isize);
	while curd < endd {
		*curd = *curs;
		curd = curd.offset(1);
		curs = curs.offset(1);
	}
	dst
}

#[no_mangle]
pub unsafe extern fn memset(ptr: *mut (), value: i32, num: usize) -> *mut () {
	let mut cur = ptr as *mut u8;
	let end = cur.offset(num as isize);
	while cur < end {
		*cur = value as u8;
		cur = cur.offset(1);
	}
	ptr
}

#[no_mangle]
#[allow(non_upper_case_globals)]
pub static floor: isize = 0;
#[no_mangle]
#[allow(non_upper_case_globals)]
pub static ceil: isize = 0;
#[no_mangle]
#[allow(non_upper_case_globals)]
pub static round: isize = 0;
#[no_mangle]
#[allow(non_upper_case_globals)]
pub static trunc: isize = 0;

#[no_mangle]
#[allow(non_upper_case_globals)]
pub static floorf: isize = 0;
#[no_mangle]
#[allow(non_upper_case_globals)]
pub static ceilf: isize = 0;
#[no_mangle]
#[allow(non_upper_case_globals)]
pub static roundf: isize = 0;
#[no_mangle]
#[allow(non_upper_case_globals)]
pub static truncf: isize = 0;

#[no_mangle]
#[allow(non_upper_case_globals)]
pub static exp: isize = 0;
#[no_mangle]
#[allow(non_upper_case_globals)]
pub static exp2: isize = 0;
#[no_mangle]
#[allow(non_upper_case_globals)]
pub static expf: isize = 0;
#[no_mangle]
#[allow(non_upper_case_globals)]
pub static exp2f: isize = 0;

#[no_mangle]
#[allow(non_upper_case_globals)]
pub static fmod: isize = 0;
#[no_mangle]
#[allow(non_upper_case_globals)]
pub static fmodf: isize = 0;


#[no_mangle]
#[allow(non_upper_case_globals)]
pub static pow: isize = 0;
#[no_mangle]
#[allow(non_upper_case_globals)]
pub static powf: isize = 0;

#[no_mangle]
#[allow(non_upper_case_globals)]
pub static __powisf2: isize = 0;
#[no_mangle]
#[allow(non_upper_case_globals)]
pub static __powidf2: isize = 0;

#[no_mangle]
#[allow(non_upper_case_globals)]
pub static log: isize = 0;
#[no_mangle]
#[allow(non_upper_case_globals)]
pub static log2: isize = 0;
#[no_mangle]
#[allow(non_upper_case_globals)]
pub static log10: isize = 0;
#[no_mangle]
#[allow(non_upper_case_globals)]
pub static logf: isize = 0;
#[no_mangle]
#[allow(non_upper_case_globals)]
pub static log2f: isize = 0;
#[no_mangle]
#[allow(non_upper_case_globals)]
pub static log10f: isize = 0;

#[no_mangle]
#[allow(non_upper_case_globals)]
pub static fma: isize = 0;
#[no_mangle]
#[allow(non_upper_case_globals)]
pub static fmaf: isize = 0;