#ifndef __AARCH64_AARCH64_H_
#define __AARCH64_AARCH64_H_

namespace toy {
namespace aarch64 {

enum class CpacrEl1 : uint32_t {
  kTrapFpSimdInE0OrE1 = 0 << 20,
  kTrapFpSimdInE0 = 1 << 20,
  kNoFpSimdTraps = 3 << 20
};

}
}

#endif // __AARCH64_AARCH64_H_
