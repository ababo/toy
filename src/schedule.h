#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "config.h"
#include "util.h"

typedef uint64_t cpu_affinity[SIZE_ELEMENTS(CONFIG_CPUS_MAX, 64)];

struct thread_context { // field order corresponds to interrupt stack layout
  uint64_t r15, r14, r13, r12, r11, r10, r9, r8, rdi, rsi, rdx, rcx, rbx, rax;
  uint8_t fxdata[512];
  struct int_stack_frame frame;
};

struct thread_desc {
  uint64_t magic;
  struct thread_context context;
  struct thread_desc *next;
  uint8_t *stack;
  size_t stack_size;
  cpu_affinity affinity;
  uint8_t priority;
  uint8_t real_priority;
  uint16_t quantum;
  uint8_t skip_store_context : 1;
  uint8_t fixed_priority : 1;
  uint8_t fixed_affinity : 1;
  uint8_t protected : 1;
};

typedef uint64_t thread_id;
typedef void (*thread_proc)(uint64_t data);

error create_thread(thread_proc proc, uint64_t data, size_t stack_size,
                    thread_id *id);
error destroy_thread(thread_id id);

error resume_thread(thread_id id);
error pause_thread(thread_id id);
error set_thread_priority(thread_id id, int priority);
error set_thread_affinity(thread_id id, const cpu_affinity *affinity);

thread_id get_thread(void);
error get_next_thread(int cpu, thread_id *id);
error get_thread_desc(thread_id id, struct thread_desc *desc);

void init_scheduler(void);

#endif // SCHEDULE_H
