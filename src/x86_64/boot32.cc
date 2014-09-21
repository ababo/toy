#include "config.h"
#include "third_party/multiboot.h"
#include "x86_64/x86_64.h"

using namespace toy;
using namespace toy::x86_64;

const multiboot_uint32_t kMultibootHeaderFlags = MULTIBOOT_MEMORY_INFO;

__attribute__((section(".mbh"), used))
const struct multiboot_header __multiboot_header = {
  MULTIBOOT_HEADER_MAGIC, kMultibootHeaderFlags,
  -(MULTIBOOT_HEADER_MAGIC + kMultibootHeaderFlags)
};

uint32_t __multiboot_info = 0;

__attribute__((aligned(16)))
uint8_t __boot_stack[kBootStackSize] = {};

namespace {

__attribute__((used))
void Stub() {
  __asm__(R"!!!(

.text

.global __bstart32
__bstart32:

  movl __boot_stack, %%esp
  addl %0, %%esp

  movl %%ebx, __multiboot_info

  movb $0xFF, %%al
  outb %%al, $0xA1
  outb %%al, $0x21 # disable IRQs

  movl %%cr4, %%edx
  orl %1, %%edx
  movl %%edx, %%cr4 # enable SSE

  call __boot32
  call __hlt

  )!!!" : : "i"(kBootStackSize), "i"(Cr4::kOsfxsr));
}

}

extern "C" void __boot32() {
  *(char*)0xB8000 = '!';

}

extern "C" void __hlt(void) {
  while(true) __asm__ __volatile__("hlt");
}
