#include <cstdint>

#include "aarch64/aarch64.h"
#include "config.h"
#include "klog.h"

using namespace toy;
using namespace toy::aarch64;

__attribute__((aligned(16)))
uint8_t __boot_stack[kBootStackSize] = {};

namespace {

__attribute__((used, section(".boot")))
void Stub () {
  __asm__(R"!!!(

        adr x1, __boot_stack
        mov x2, %0
        add sp, x1, x2

        mov x0, %1
        msr cpacr_el1, x0 // disable FP and SIMD traps

        bl __boot
        bl __wfi

  )!!!" : : "i"(kBootStackSize), "i"(CpacrEl1::kNoFpSimdTraps));
}

void putc(uint32_t chr) {
  volatile char* const kUartAddr =
      reinterpret_cast<char*>(0x9000000);
  *kUartAddr = static_cast<char>(chr);
}

}

extern "C" void __boot(void) {
  klog.Initialize(putc, KLogLevel::kInfo);
  klog.Info("Hello from %s!", "kernel");

}

extern "C" void __wfi(void) {
  while(true) __asm__ __volatile__("wfi");
}

extern "C" void abort(void) {
  klog.Error("The system is stopped: 'abort' was called");
  __wfi();
}

extern "C" void* __cxa_begin_catch(void* exceptionObject) {
  abort(); // not implemented yet
  return exceptionObject;
}
