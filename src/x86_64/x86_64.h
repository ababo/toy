#ifndef __X86_64_X86_64_H_
#define __X86_64_X86_64_H_

#include <cstdint>

#include "config.h"

namespace {

inline uint64_t GetMsr(uint32_t msr) {
  uint32_t low, high;
  __asm__("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
  return (static_cast<uint64_t>(high) << 32) + low;
}

inline void SetMsr(uint32_t msr, uint64_t value) {
  uint32_t low = static_cast<uint32_t>(value);
  uint32_t high = static_cast<uint32_t>(value >> 32);
  __asm__("wrmsr" : : "a"(low), "d"(high), "c"(msr));
}

}

namespace toy {
namespace x86_64 {

enum Port : uint16_t {
  kPic1Code = 0x20,
  kPic1Data = 0x21,
  kPic2Code = 0xA0,
  kPic2Data = 0xA1
};

enum Cr0 : uint32_t {
  kPe = 1 << 0,
  kMp = 1 << 1,
  kEm = 1 << 2,
  kTs = 1 << 3,
  kEt = 1 << 4,
  kNe = 1 << 5,
  kWp = 1 << 16,
  kAm = 1 << 18,
  kNw = 1 << 29,
  kCd = 1 << 30,
  kPg = 1LL << 31
};

enum Cr4 : uint32_t {
  kVme = 1 << 0,
  kPvi = 1 << 1,
  kTsd = 1 << 2,
  kDe = 1 << 3,
  kPse = 1 << 4,
  kPae = 1 << 5,
  kMce = 1 << 6,
  kPge = 1 << 7,
  kPce = 1 << 8,
  kOsfxsr = 1 << 9,
  kOsxmmexcpt = 1 << 10,
  kVmxe = 10 << 13,
  kSmxe = 10 << 14,
  kPcide = 1 << 17,
  kOsxsave = 1 << 18,
  kSmep = 1 << 20,
  kSmap = 1 << 21
};

enum Msr : uint32_t {
  kEfer = 0xC0000080
};

enum MsrEfer : uint64_t {
  kLme = 1 << 8,
  kLma = 1 << 10,
  kNxe = 1 << 11,
  kSvme = 1 << 12,
  kLmsle = 1 << 13,
  kFfxsr = 1 << 14,
  kTce = 1 << 15
};

const unsigned kPageTableEntries = 512;

struct PageEntry {
  PageEntry* ChildTable() const volatile {
    return reinterpret_cast<PageEntry*>(Address());
  }
  void SetChildTable(volatile PageEntry* child_entries) volatile {
    SetAddress(reinterpret_cast<uint64_t>(child_entries));
  }

  uint64_t Address() const volatile {
    return static_cast<uint64_t>(address1) << 32 |
        static_cast<uint64_t>(address0) << 12;
  }
  void SetAddress(uint64_t address) volatile {
    address0 = static_cast<uint32_t>(address >> 12);
    address1 = static_cast<uint32_t>(address >> 32);
  }

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

struct TableInfo {
  uint16_t limit;
  uint64_t base;
} __attribute__((packed));

struct GdtEntry {
  enum Type : uint8_t {
    kData = 0x2, kTss = 0x9, kCode = 0xA
  };

  uint16_t limit0;
  uint16_t base0;
  uint8_t base1;
  uint8_t type : 4;
  uint8_t nonsys : 1;
  uint8_t dpl : 2;
  uint8_t present : 1;
  uint8_t limit1 : 4;
  uint8_t avl : 1;
  uint8_t bits64 : 1;
  uint8_t bits32 : 1;
  uint8_t gran : 1;
  uint8_t base2;
};

struct GdtEntryEx : GdtEntry {
  uint32_t base3;
  uint32_t reserved0 : 8;
  uint32_t zero : 5;
  uint32_t reserved1 : 19;
};

struct ToyGdtTable {
  enum Segment : uint16_t {
    kCode = 0x8, kData = 0x10
  };

  GdtEntry zero, code, data;
  GdtEntryEx cpus[kMaxCpus];
};

}
}

#endif  // __X86_64_X86_64_H_
