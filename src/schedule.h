#ifndef __SCHEDULE_H
#define __SCHEDULE_H

#include "common.h"
#include "config.h"
#include ARCH_FILE(/schedule.inc)

#define THREAD_STACK_SIZE_MIN 16

#define THREAD_AFFINITY_SIZE SIZE_ELEMENTS(CONFIG_CPUS_MAX, 64)

#define THREAD_STATE_UNKNOWN 0
#define THREAD_STATE_RUNNING 1
#define THREAD_STATE_PAUSED 2
#define THREAD_STATE_STOPPED 3

struct spinlock;
// typedef ... thread_id;
// typedef ... thread_context;
typedef uint64_t (*thread_proc)(uint64_t input);

// IN fields should be set before calling attach_thread
struct thread_data {
  INTERNAL uint64_t magic;
  INTERNAL struct thread_data *prev, *next, *all_prev, *all_next;
  IN thread_context context;
  IN uint8_t *stack;
  IN size_t stack_size; // size available to thread: stack_size - 8
  IN uint64_t affinity[THREAD_AFFINITY_SIZE];
  OUT uint64_t output;
  OUT uint64_t run_time;
  IN uint8_t priority;
  INTERNAL uint8_t real_priority;
  INTERNAL uint16_t quantum;
  INTERNAL uint8_t cpu;
  INTERNAL uint8_t state: 2;
  IN uint8_t fixed_priority : 1;
};

// set stack and stack_size fields before calling this function
err_code set_thread_context(struct thread_data *thread, thread_proc proc,
                            uint64_t input);

// attached thread becomes paused; thread should not reside in stack!
err_code attach_thread(struct thread_data *thread, thread_id *id);
err_code detach_thread(thread_id id, struct thread_data **thread);

err_code resume_thread(thread_id id);
err_code pause_thread(thread_id id);
err_code stop_thread(thread_id id, uint64_t output);

// a given spinlock is released atomically (within an interrupt)
err_code pause_this_thread(struct spinlock *lock_to_release);

static inline thread_id get_thread(void);

typedef void (*timer_proc)(uint64_t ticks);

uint64_t get_ticks(void);
timer_proc get_timer_proc(void);
uint64_t get_timer_ticks(void); // is zeroed after triggering
void set_timer_proc(timer_proc proc); // called within a timer interrupt
void set_timer_ticks(uint64_t ticks); // not thread-safe

void init_scheduler(void);

#endif // __SCHEDULE_H
