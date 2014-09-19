# Toy OS (version 0.4)
###Simple OS-like program for x86-64 and AArch64 which dreams to become a real OS.

The system is under development now. See [my blog](http://ababo.github.io/toy) and [the previous version](https://github.com/ababo/toy/tree/ver.0.3).

####Build and run instructions for Ubuntu 14.04 LTS.

1. Install binary tools:
   * *sudo apt-get install binutils binutils-aarch64-linux-gnu*

2. Install GCC/CLang compilers for x86_64 and AArch64:
   * *sudo apt-get install g++ g++-aarch64-linux-gnu*
   * *sudo apt-get install clang*

3. Install ncurses (to make qemu-system-x86_64 able to run in terminal).
   * *sudo apt-get install ncurses-dev*

4. Build and install QEMU 2.1.0 or higher to get qemu-system-aarch64:
   * Download the latest version from [here](http://wiki.qemu.org/Download).
   * Unpack the archive contents and cd into its root directory.
   * *./configure --target-list=x86_64-softmmu,aarch64-softmmu --enable-curses*
   * *make && sudo make install*

5. Install xorriso (this tool is needed for making bootable ISO disks):
   * *sudo apt-get install xorriso*

6. Cd into the project directory and configure it:
   * *./waf configure -a {ARCH} -c {COMP}*
     * Here {ARCH} can be 'x86_64' or 'aarch64'; {COMP} can be 'gcc' or 'clang'.

7. Build and run in emulator:
   * *./waf build run*
