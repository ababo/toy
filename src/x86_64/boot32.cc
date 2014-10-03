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

__attribute__((aligned(4096)))
PageEntry __pml4[kPageTableEntries + 1] = {};

__attribute__((aligned(4)))
ToyGdtTable __gdt = {};

namespace {

__attribute__((used))
void Stub() {
  __asm__(R"!!!(

        .text
        .global __start32
__start32:
        movl %%ebx, __multiboot_info

        movb $0xFF, %%al
        outb %%al, %0
        outb %%al, %1 # disable IRQs

        movl %%cr4, %%edx
        orl %2, %%edx
        movl %%edx, %%cr4 # enable PAE and SSE

        movl $__pml4, %%eax
        movl %%eax, %%cr3 # assign page map

        movl __boot_stack, %%esp
        addl %3, %%esp
        call __boot32
halt:
        hlt
        jmp halt

  )!!!" : : "i"(Port::kPic1Data)
          , "i"(Port::kPic2Data)
          , "i"(Cr4::kPae | Cr4::kOsfxsr)
          , "i"(kBootStackSize));
}

}

extern "C" void __boot32() {
  // map 1:1 first 1GiB of RAM
  volatile PageEntry& pml4e = __pml4[0];
  volatile PageEntry& pde = __pml4[kPageTableEntries];
  pml4e.present = 1;
  pml4e.write = 1;
  pml4e.SetChildTable(&pde);
  pde.present = 1;
  pde.write = 1;
  pde.ps_pat = 1;

  // prepare GDT
  __gdt.code.type = GdtEntry::Type::kCode;
  __gdt.code.nonsys = 1;
  __gdt.code.present = 1;
  __gdt.code.bits64 = 1;
  __gdt.data.type = GdtEntry::Type::kData;
  __gdt.data.nonsys = 1;
  __gdt.data.present = 1;
  __gdt.data.bits32 = 1;

  TableInfo gdti;
  gdti.limit = sizeof(ToyGdtTable) - 1;
  gdti.base = reinterpret_cast<uint64_t>(&__gdt);

  // enable long mode
  SetMsr(Msr::kEfer, GetMsr(Msr::kEfer) | MsrEfer::kLme);

  __asm__(R"!!!(

        movl %%cr0, %%eax
        orl %0, %%eax
        movl %%eax, %%cr0 # enable paging

        lgdt %1 # load GDT

        ljmp %2, $__start # jump to 64-bit code

  )!!!" : : "i"(Cr0::kPg)
          , "m"(gdti)
          , "i"(ToyGdtTable::Segment::kCode));
}
