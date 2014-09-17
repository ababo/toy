# Toy OS (version 0.4)
###Simple OS-like program for x86-64 and AArch64 which dreams to become a real OS.

The system is under development now. See [my blog](http://ababo.github.io/toy) and [the previous version](https://github.com/ababo/toy/tree/ver.0.3).

####Build and run instructions for Ubuntu 14.04 LTS.

**x86_64 target:**

Not ready yet, see [the previous version](https://github.com/ababo/toy/tree/ver.0.3)

**AArch64 target:**

1. Binary tools:
   * *sudo apt-get install binutils-aarch64-linux-gnu*

2. GCC cross compiler for AArch64 or/and CLang:
   * *sudo apt-get install g++-aarch64-linux-gnu*
   * *sudo apt-get install clang*

3. Build and install QEMU 2.1.0 or higher to get qemu-system-aarch64.

4. Cd into the project directory and configure it:
   * *./waf configure -a aarch64 -c gcc* (using GCC) or
   * *./waf conigure -a aarch64 -c clang* (using CLang)

5. Build and run it in emulator:
   * *./waf run*
