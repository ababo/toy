#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "config.h"
#include "util.h"

struct thread_context {
  uint64_t rax, rbx, rcx, rdx, rbp, rsi, rdi, rsp;
  uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
  uint64_t rip, rflags;
  uint8_t fxdata[512];
};

struct thread_desc {
  uint64_t magic;
  size_t size;
  struct thread_desc *prev, *next;
  struct thread_context context;
  uint64_t work_time; // in microseconds
  uint8_t cpu;
  uint8_t priority;
  uint16_t paused : 1;
  uint16_t reserved0 : 15;
  uint32_t reserved1;
  uint64_t stack_overrun_magic;
  uint8_t stack[];
};

struct scheduler_stats {
  uint8_t busy_percentage[CONFIG_CPUS_MAX];
};

typedef uint64_t thread_id;
typedef void (*thread_proc)(uint64_t data);

error create_thread(thread_proc proc, uint64_t data, size_t stack_size,
                    thread_id *id);
error destroy_thread(thread_id id);

error resume_thread(thread_id id);
error pause_thread(thread_id id);
error set_thread_priority(thread_id id, int priority);

error get_thread_desc(thread_id id, struct thread_desc *desc);
error get_scheduler_stats(struct scheduler_stats *stats);

void init_scheduler(void);

#endif // SCHEDULE_H
