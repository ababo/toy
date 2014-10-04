#include <cstdlib>

#include "config.h"
#include "klog.h"
#include "x86_64/vga.h"
#include "x86_64/x86_64.h"

using namespace toy;
using namespace toy::x86_64;

namespace {

__attribute__((used))
void Stub() {
  asm(R"!!!(

        .global __start
__start:
        movw %0, %%ax
        movw %%ax, %%ds
        movw %%ax, %%ss

        movq __boot_stack, %%rsp
        addq %1, %%rsp
        call __boot
        call __halt

  )!!!" : : "i"(ToyGdtTable::Segment::kData), "i"(kBootStackSize));
}

}

extern "C" void __boot(void) {
  Vga::Initialize(Vga::Color::kLightGray, Vga::Color::kBlack);
  klog.Initialize(Vga::Putc, KLog::Level::kInfo);
  klog.Info("Hello from %s!", "kernel");

}

extern "C" void __halt(void) {
  while(true) asm volatile("hlt");
}

extern "C" void* __cxa_begin_catch(void* exceptionObject) {
  abort(); // not implemented yet
  return exceptionObject;
}
