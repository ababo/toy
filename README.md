#### Building instructions:
1. Install Rust Nightly from [here](http://www.rust-lang.org/install.html).
2. Install binutils for supported targets:
	* Mac OS instructions:
		1. Install MacPorts from [here](https://www.macports.org/install.php).
    	2. Run "sudo port install x86_64-elf-binutils ~~aarch64-elf-binutils~~".
		3. Since there is still no aarch64-elf-binutils package build binutils manually:
			1. Download binutils from [here](http://ftp.gnu.org/gnu/binutils/).
			2. Unpack and cd into the directory.
			3. Run "./configure --target aarch64-elf && make && sudo make install"
	* Ubuntu instructions:
		1. For non-x86_64 system run "sudo apt-get install binutils-x86_64-linux-gnu".
		2. For non-aarch64 system run "sudo apt-get install binutils-aarch64-linux-gnu".
3. Install QEMU 2.2+:
	* Mac OS instructions:
		1. Run: "sudo port install qemu +target_x86_64 +target_arm".
	* Ubuntu instructions:
		1. ~~Run "sudo apt-get install qemu-system-x86 qemu-system-aarch64"~~.
		2. Since Ubuntu 14.04 comes with QEMU 2.0 which doesn't yet support aarch64 build latest QEMU manually:
			1. Run "sudo apt-get install g++ libglib2.0-dev zlib1g-dev autoconf libtool".
			2. Download QEMU from [here](http://wiki.qemu.org/Download).
			3. Unpack and cd into the directory.
			3. Run "./configure --target-list=x86_64-softmmu,aarch64-softmmu --enable-system --disable-user && make && sudo make install".
4. Build and run Toy OS:
	1. Cd into the directory.
	2. For x86_64 target run: "make run ARCH=x86_64".
	3. For aarch64 target run: "make run ARCH=aarch64".