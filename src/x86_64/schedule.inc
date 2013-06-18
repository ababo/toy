#include "../sync.h"
#include "apic.h"
#include "cpu.h"
#include "interrupt.h"

typedef struct int_stack_frame thread_context;
typedef stamp_id thread_id;
struct thread_data;

static inline const struct thread_data *__get_thread(void) {
  return (struct thread_data*)rdmsr(MSR_FS_BASE);
}

static inline void __set_thread(const struct thread_data *thread) {
  wrmsr(MSR_FS_BASE, (uint64_t)thread);
}

static inline void __load_thread_context(struct int_stack_frame *stack_frame,
                                         thread_context *context) {
  *context = *stack_frame;
}

static inline void __store_thread_context(struct int_stack_frame *stack_frame,
                                          thread_context *context) {
  *stack_frame = *context;
}

static inline void __store_halt(struct int_stack_frame *stack_frame) {
  extern int __halt;
  stack_frame->rip = (uint64_t)&__halt;
}

static inline void __store_int_enabled(struct int_stack_frame *stack_frame) {
  stack_frame->rflags |= RFLAGS_IF;
}

static inline int __find_mask_bsf(const uint64_t *mask, int size) {
  for (int i = 0; i < size; i++)
    if (mask[i])
      return i * 64 + bsfq(mask[i]);
  return -1;
}

static inline void __issue_task_interrupt(int cpu) {
  issue_cpu_interrupt(cpu, INT_VECTOR_SCHEDULER_TASK);
}

static inline void __issue_local_task_interrupt(void) {
  ASMV("int %0" : : "i"(INT_VECTOR_SCHEDULER_TASK));
}

static inline void __set_eoi(void) {
  set_apic_eoi();
}
