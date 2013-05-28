#include "schedule.h"

// #define __READ_THREAD_CONTEXT(context) ...
// #define __WRITE_THREAD_CONTEXT(context) ...

static inline void __set_eoi(void);
static inline int __find_mask_bsf(const uint64_t *mask, int size);

void __issue_task_interrupt(int cpu);

err_code attach_thread(struct thread_data *thread, thread_id *id) {

}

err_code detach_thread(thread_id id, struct thread_data **thread) {

}

err_code resume_thread(thread_id id) {

}

err_code pause_thread(thread_id id) {

}

err_code stop_thread(thread_id id, uint64_t output) {

}

err_code pause_this_thread(struct spinlock *lock_to_release) {

}

uint64_t get_ticks(void) {

}

timer_proc get_timer_proc(void) {

}

uint64_t get_timer_ticks(void) {

}

void set_timer_proc(timer_proc proc) {

}

void set_timer_ticks(uint64_t ticks) {

}

void init_scheduler(void) {

}
