//
// x86-64-specific CPU constants and structures.
//

#ifndef __X86_64_CPU_H_
#define __X86_64_CPU_H_

#define CR4_PAE (1 << 5)
#define CR4_OSFXSR (1 << 9)

#define PAGE_TABLE_ENTRIES 512

#ifndef __ASSEMBLER__

#include "common.h"

struct page_entry {
  uint32_t present : 1;
  uint32_t write : 1;
  uint32_t nonsys : 1;
  uint32_t pwt : 1;
  uint32_t pcd : 1;
  uint32_t accessed : 1;
  uint32_t dirty : 1;
  uint32_t ps_pat : 1;
  uint32_t global : 1;
  uint32_t avail0 : 3;
  uint32_t address0 : 20;
  uint32_t address1 : 20;
  uint32_t avail1 : 11;
  uint32_t noexec : 1;
};

#endif // __ASSEMBLER__

#endif // __X86_64_CPU_H_
