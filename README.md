##### Building instructions:
1. Install Rust Nightly from [here](http://www.rust-lang.org/install.html).
2. Install binutils for supported targets:
	* Mac OS instructions:
		1. Install MacPorts from [here](https://www.macports.org/install.php).
    	2. Run "sudo port install x86_64-elf-binutils aarch64-elf-binutils".
	* Ubuntu instructions:
		1. For non-x86_64 system run "sudo apt-get install binutils-x86_64-linux-gnu".
		2. For non-aarch64 system run "sudo apt-get install binutils-aarch64-linux-gnu".
3. Install QEMU:
	* Mac OS instructions:
		1. Run: "sudo port install qemu +target_x86_64 +target_arm".
	* Ubuntu instructions:
		1. Run "sudo apt-get install qemu-system-x86_64 qemu-system-arm".