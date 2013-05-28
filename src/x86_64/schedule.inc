#include "apic.h"
#include "cpu.h"
#include "interrupt.h"

typedef struct int_stack_frame thread_context;
typedef uint64_t thread_id;

static inline thread_id get_thread(void) {
  return rdmsr(MSR_FS_BASE);
}

#define __READ_THREAD_CONTEXT(context) context = *stack_frame;
#define __WRITE_THREAD_CONTEXT(context) *stack_frame = context;

static inline void __set_eoi(void) {
  set_apic_eoi();
}

static inline int __find_mask_bsf(const uint64_t *mask, int size) {
  for (int i = 0; i < size; i++)
    if (mask[i])
      return i * 64 + bsfq(mask[i]);
  return -1;
}
