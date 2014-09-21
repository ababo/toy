#ifndef __X86_64_X86_64_H_
#define __X86_64_X86_64_H_

#include <cstdint>

namespace toy {
namespace x86_64 {

enum class Cr4 : uint32_t {
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

}
}

#endif  // __X86_64_X86_64_H_
