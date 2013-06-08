#include "../config.h"
#include "../cpu_info.h"
#include "../schedule.h"
#include "apic.h"
#include "cpu.h"
#include "interrupt.h"

#define STACK_OVERRUN_MAGIC 0x2384626433832795

ASM(".global __exit_thread; __exit_thread:;"
    "pushq %rax; movl $" STR_EXPAND(MSR_FS_BASE) ", %ecx;"
    "rdmsr; movl %edx, %edi; shll $32, %edi; movl %eax, %edi;"
    "popq %rsi; callq stop_thread");

err_code set_thread_context(struct thread_data *thread, thread_proc proc,
                            uint64_t input) {
  if (!thread->stack || thread->stack_size < THREAD_STACK_SIZE_MIN)
    return ERR_BAD_INPUT;

  extern int __exit_thread;
  *(uint64_t*)thread->stack = STACK_OVERRUN_MAGIC;
  uint64_t *top = (uint64_t*)(thread->stack + thread->stack_size - 8);
  *top = (uint64_t)&__exit_thread;

  thread->context.rflags = RFLAGS_IF;
  thread->context.cs = SEGMENT_CODE;
  thread->context.ss = SEGMENT_DATA;
  thread->context.rsp = (uint64_t)top;
  thread->context.rip = (uint64_t)proc;
  thread->context.rdi = input;
  return ERR_NONE;
}

void __panic_stack_overrun(struct int_stack_frame *stack_frame,
                           int cpu, struct thread_data *thread) {
  ASMV("cli");
  set_outer_spinlock(true);
  kprintf("\nstack overrun (CPU: %d, thread: %lX):\n",
          cpu, (uint64_t)thread);
  dump_int_stack_frame(stack_frame);
  ASMV("jmp __halt");
}

extern void *__get_timer_isr(void);
extern void *__get_task_isr(void);

void __init_scheduler(void) {
  if (get_cpu() == get_bsp_cpu()) {
    set_isr(INT_VECTOR_APIC_TIMER, __get_timer_isr());
    set_isr(INT_VECTOR_SCHEDULER_TASK, __get_task_isr());
  }

  start_apic_timer(CONFIG_SCHEDULER_TICK_INTERVAL, true);
}
