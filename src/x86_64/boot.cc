#include "config.h"
#include "x86_64/x86_64.h"

using namespace toy;
using namespace toy::x86_64;

namespace {

__attribute__((used))
void Stub() {
  __asm__(R"!!!(

        .global __start
__start:
        movw %0, %%ax
        movw %%ax, %%ds
        movw %%ax, %%ss

        movq __boot_stack, %%rsp
        addq %1, %%rsp
        call __boot
        call __halt

  )!!!" : : "i"(ToyGdtTable::Segment::kData)
          , "i"(kBootStackSize));
}

}

extern "C" void __boot(void) {
  *(char*)0xB8000 = '!';
}

extern "C" void __halt(void) {
  while(true) __asm__ __volatile__("hlt");
}

extern "C" void abort(void) {
  __halt();
}

extern "C" void* __cxa_begin_catch(void* exceptionObject) {
  abort(); // not implemented yet
  return exceptionObject;
}
