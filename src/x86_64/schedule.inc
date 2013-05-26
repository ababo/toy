#include "cpu.h"
#include "interrupt.h"

typedef struct int_stack_frame thread_context;
typedef uint64_t thread_id;

static inline thread_id get_thread(void) {
  return rdmsr(MSR_FS_BASE);
}
