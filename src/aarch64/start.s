	.set CPACR_EL1_FPEN, 0b11 << 20

	.set BOOT_STACK_SIZE, 8 * 1024

	.global __boot_stack
	.global __start
	.global __halt

	.bss
	.align 8
__boot_stack:
	.fill BOOT_STACK_SIZE

	.text
__start:
	/* disable FP and SIMD traps */
	mov x0, #CPACR_EL1_FPEN
	msr cpacr_el1, x0

	/* set stack */
	adr x0, __boot_stack
	add sp, x0, #BOOT_STACK_SIZE

	/* call the Rust entry point */
	bl __boot

__halt:
	/* halt CPU */
	wfi
	b __halt
