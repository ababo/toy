#include "cpu_info.h"
#include "schedule.h"
#include "sync.h"

// #define DEFINE_INT_HANDLER(name) ...
// #define DEFINE_ISR(name, data) ...

struct int_stack_frame;

static inline void __set_thread(const struct thread_data *thread);
static inline void __load_thread_context(struct int_stack_frame *stack_frame,
                                         thread_context *context);
static inline void __store_thread_context(struct int_stack_frame *stack_frame,
                                          thread_context *context);
static inline void __store_halt(struct int_stack_frame *stack_frame);
static inline void __store_int_enabled(struct int_stack_frame *stack_frame);
static inline int __find_mask_bsf(const uint64_t *mask, int size);
static inline void __issue_task_interrupt(int cpu);
static inline void __issue_local_task_interrupt(void);
static inline void __set_eoi(void);

void __panic_stack_overrun(struct int_stack_frame *stack_frame,
                           int cpu, struct thread_data *thread);
void __init_scheduler(void);

#define THREAD_MAGIC 0x3141592653589793
#define STACK_OVERRUN_MAGIC 0x2384626433832795

struct cpu_priority {
  struct thread_data *expired_head, *expired_tail, *active_tail;
  int total_threads;
};

#define CPU_TASK_RESUME 0
#define CPU_TASK_PAUSE 1
#define CPU_TASK_SELF_HALT 2

struct cpu_task {
  IN uint8_t type;
  OUT volatile err_code error;
  IN struct thread_data *thread;
  IN struct spinlock *lock;
};

#define PRIORITY_MASK_SIZE SIZE_ELEMENTS(CONFIG_SCHEDULER_PRIORITIES, 64)

static struct cpu_data {
  int total_threads;
  int current_priority;
  struct cpu_priority priorities[CONFIG_SCHEDULER_PRIORITIES];
  uint64_t priority_mask[PRIORITY_MASK_SIZE];
  struct cpu_task task;
  struct spinlock lock;
  bool load_context;
} cpus[CONFIG_MAX_CPUS];

static struct thread_list {
  struct spinlock lock;
  int total_threads;
  struct thread_data *tail;
} all, inactive;

static int stamp;

err_code attach_thread(struct thread_data *thread, thread_id *id) {
  if (thread->magic == THREAD_MAGIC) // already attached
    return ERR_BAD_STATE;

  if (!thread->stack || thread->stack_size < THREAD_STACK_SIZE_MIN ||
      *(uint64_t*)thread->stack != STACK_OVERRUN_MAGIC ||
      thread->priority >= CONFIG_SCHEDULER_PRIORITIES)
    return ERR_BAD_INPUT;

  int bsf = __find_mask_bsf(thread->affinity, THREAD_AFFINITY_SIZE);
  if (bsf == -1 || bsf >= get_cpus())
    return ERR_BAD_INPUT;

  thread->prev = thread->all_prev = NULL;
  thread->state = THREAD_STATE_PAUSED;
  thread->run_time = 0;

  acquire_spinlock(&all.lock, 0);
  acquire_spinlock(&inactive.lock, 0);

  thread->magic = THREAD_MAGIC;
  thread->stamp = stamp;
  stamp = (stamp + 1) % (1 << PACKED_POINTER_DATA_BITS);
  thread->all_next = all.tail;
  if (all.tail)
    all.tail->all_prev = thread;
  all.tail = thread;
  all.total_threads++;

  thread->next = inactive.tail;
  if (inactive.tail)
    inactive.tail->prev = thread;
  inactive.tail = thread;
  inactive.total_threads++;

  release_spinlock(&inactive.lock);
  release_spinlock(&all.lock);

  if (id)
    *id = pack_pointer(thread, thread->stamp);
  return ERR_NONE;
}

#define DO_THREAD(thrd, id)                              \
  int stamp;                                             \
  struct thread_data *thrd = unpack_pointer(id, &stamp); \
  if (thrd->magic != THREAD_MAGIC)                       \
    err = ERR_NOT_FOUND;                                 \
  else if (thrd->stamp != stamp)                         \
    err = ERR_EXPIRED;                                   \
  else

err_code detach_thread(thread_id id, struct thread_data **thread) {
  err_code err = ERR_NONE;
  acquire_spinlock(&all.lock, 0);
  acquire_spinlock(&inactive.lock, 0);

  DO_THREAD(thrd, id) {
    if (thrd->state == THREAD_STATE_PAUSED ||
        thrd->state == THREAD_STATE_STOPPED) {
      if (thrd->next)
        thrd->next->prev = thrd->prev;
      if (thrd->prev)
        thrd->prev->next = thrd->next;
      if (inactive.tail == thrd)
        inactive.tail = thrd->next;
      inactive.total_threads--;

      if (thrd->all_next)
        thrd->all_next->all_prev = thrd->all_prev;
      if (thrd->all_prev)
        thrd->all_prev->all_next = thrd->all_next;
      if (all.tail == thrd)
        all.tail = thrd->all_next;
      all.total_threads--;

      thrd->magic = 0;
      thrd->prev = thrd->next = NULL;
      thrd->all_prev = thrd->all_next = NULL;
      *thread = thrd;
    }
    else
      err = ERR_BAD_STATE;
  }

  release_spinlock(&inactive.lock);
  release_spinlock(&all.lock);
  return err;
}

static inline void update_priority_quantum(struct thread_data *thread) {
  /// TODO: make priority and quantum change dynamically
  thread->real_priority = thread->priority;
  thread->quantum = 10;
}

static int find_least_loaded_cpu(uint64_t *affinity) {
  // no need to lock here, just find a minimum
  int cpu = -1, total = INT32_MAX;
  for (int i = 0; i < get_cpus(); i++)
    if (BIT_ARRAY_GET(affinity, i)) {
      int temp = cpus[i].total_threads;
      if (temp < total)
        cpu = i, total = temp;
    }
  return cpu;
}

DEFINE_INT_HANDLER(task);

static void run_cpu_task(int cpu, struct cpu_task *task) {
  struct cpu_data *cpud = &cpus[cpu];
  acquire_spinlock(&cpud->lock, 0);
  cpud->task = *task;
  if (get_cpu() == cpu)
    __handle_task_int(NULL, 0);
  else {
    cpud->task.error = ERR_BUSY;
    __issue_task_interrupt(cpu);
    while (cpud->task.error == ERR_BUSY) { }
  }
  *task = cpud->task;
  if (task->type != CPU_TASK_SELF_HALT)
    release_spinlock(&cpud->lock);
}

err_code resume_thread(thread_id id) {
  err_code err = ERR_NONE;
  acquire_spinlock(&inactive.lock, 0);

  DO_THREAD(thrd, id) {
    if (thrd->state == THREAD_STATE_PAUSED) {
      if (thrd->next)
        thrd->next->prev = thrd->prev;
      if (thrd->prev)
        thrd->prev->next = thrd->next;
      if (inactive.tail == thrd)
        inactive.tail = thrd->next;
      inactive.total_threads--;

      thrd->real_priority = thrd->priority;
      thrd->quantum = 0;
      update_priority_quantum(thrd);

      struct cpu_task task = { .type = CPU_TASK_RESUME, .thread = thrd };
      run_cpu_task(find_least_loaded_cpu(thrd->affinity), &task);
    }
    else
      err = ERR_BAD_STATE;
  }

  release_spinlock(&inactive.lock);
  return err;
}

static err_code deactivate_running(bool stop, struct thread_data *thread,
                                   uint64_t output,
                                   struct spinlock *lock_to_release,
                                   bool *self_halt) {
  struct cpu_task task = {
    .type = CPU_TASK_PAUSE, .thread = thread, .lock = lock_to_release
  };
  run_cpu_task(thread->cpu, &task);
  if (task.error)
    return task.error;

  thread->prev = NULL;
  thread->state = stop ? THREAD_STATE_STOPPED : THREAD_STATE_PAUSED;
  thread->next = inactive.tail;
  if (inactive.tail)
    inactive.tail->prev = thread;
  inactive.tail = thread;
  inactive.total_threads++;
  if (stop)
    thread->output = output;

  if (task.type == CPU_TASK_SELF_HALT) {
    __issue_local_task_interrupt();
    *self_halt = true;
  }

  return ERR_NONE;
}

#define TASK_RETRY_COUNT 3

static err_code deactivate_thread(bool stop, thread_id id, uint64_t output,
                                  struct spinlock *lock_to_release) {
  bool self_halt = false;
  err_code err = ERR_NONE;
  acquire_spinlock(&inactive.lock, 0);

  DO_THREAD(thrd, id) {
    for (int i = 0; i < TASK_RETRY_COUNT; i++) {
      switch (thrd->state) {
      case THREAD_STATE_RUNNING:
        err = deactivate_running(stop, thrd, output,
                                 lock_to_release, &self_halt);
        break;

      case THREAD_STATE_PAUSED:
        if (stop) {
          thrd->state = THREAD_STATE_STOPPED;
          thrd->output = output;
          err = ERR_NONE;
        }
        else
          err = ERR_BAD_STATE;
        break;

      case THREAD_STATE_STOPPED:
        err = ERR_BAD_STATE;
        break;
      }

      if (err != ERR_NOT_FOUND)
        break;
    }

    if (err == ERR_NOT_FOUND)
      err = ERR_BUSY;
  }

  if (!self_halt)
    release_spinlock(&inactive.lock);
  return err;
}

err_code pause_thread(thread_id id) {
  return deactivate_thread(false, id, 0, NULL);
}

err_code stop_thread(thread_id id, uint64_t output) {
  return deactivate_thread(true, id, output, NULL);
}

err_code pause_this_thread(struct spinlock *lock_to_release) {
  return deactivate_thread(false, get_thread(), 0, lock_to_release);
}

static uint64_t ticks, timer_ticks[CONFIG_MAX_CPUS];
static timer_proc timer_proc_;

uint64_t get_ticks(void) {
  return ticks;
}

timer_proc get_timer_proc(void) {
  return timer_proc_;
}

uint64_t get_timer_ticks(void) {
  return timer_ticks[get_cpu()];
}

void set_timer_proc(timer_proc proc) {
  timer_proc_ = proc;
}

void set_timer_ticks(uint64_t ticks) {
  timer_ticks[get_cpu()] = ticks;
}

static inline void add_expired(struct cpu_data *cpud,
                               struct thread_data *thread) {
  struct cpu_priority *prio = &cpud->priorities[thread->real_priority];
  thread->prev = thread->next = NULL;

  if (prio->total_threads) {
    if (prio->expired_head) {
      prio->expired_head->next = thread;
      thread->prev = prio->expired_head;
    }
    else
      prio->expired_tail = thread;

    prio->expired_head = thread;
  }
  else
    prio->active_tail = thread;

  BIT_ARRAY_SET(cpud->priority_mask, thread->real_priority);
  prio->total_threads++;
}

static inline void handle_timer(int cpu) {
  if (cpu == get_bsp_cpu())
    ticks++;

  if (timer_ticks[cpu] && timer_ticks[cpu] <= ticks) {
    uint64_t prev_ticks = timer_ticks[cpu];
    timer_ticks[cpu] = 0;
    set_outer_spinlock(true);
    timer_proc_(prev_ticks);
    set_outer_spinlock(false);
  }
}

DEFINE_ISR(timer, 0) {
  int cpu = get_cpu();
  struct cpu_data *cpud = &cpus[cpu];

  if (!cpud->total_threads)
    goto exit;

  struct cpu_priority *prio = &cpud->priorities[cpud->current_priority];
  struct thread_data *thrd = prio->active_tail;

  if (cpud->load_context) {
    __store_thread_context(stack_frame, &thrd->context);
    __set_thread(thrd);
    cpud->load_context = false;
    goto exit;
  }

  if (*(volatile uint64_t*)thrd->stack != STACK_OVERRUN_MAGIC)
    __panic_stack_overrun(stack_frame, cpu, thrd);

  thrd->run_time++;

  int highest = __find_mask_bsf(cpud->priority_mask, PRIORITY_MASK_SIZE);
  if (highest >= cpud->current_priority && thrd->quantum) {
    thrd->quantum--;
    goto exit;
  }

  prio->active_tail = thrd->next;
  if (!--prio->total_threads)
    BIT_ARRAY_RESET(cpud->priority_mask, cpud->current_priority);

  __load_thread_context(stack_frame, &thrd->context);
  update_priority_quantum(thrd);
  add_expired(cpud, thrd);

  if (prio->active_tail)
    prio->active_tail->prev = NULL;
  else {
    prio->active_tail = prio->expired_tail;
    prio->expired_tail = prio->expired_head = NULL;
  }

  prio = &cpud->priorities[cpud->current_priority = highest];
  struct thread_data *thrd2 = prio->active_tail;
  if (thrd2 != thrd) {
    __store_thread_context(stack_frame, &thrd2->context);
    __set_thread(thrd2);
  }

exit:
  handle_timer(cpu);
  __set_eoi();
}

static inline void do_resume_task(int cpu, struct cpu_data *cpud,
                                  struct cpu_task *task) {
  struct thread_data *thread = task->thread;
  thread->state = THREAD_STATE_RUNNING;
  thread->cpu = cpu;
  add_expired(cpud, thread);

  if (!cpud->total_threads++) {
    cpud->current_priority = thread->real_priority;
    cpud->load_context = true;
  }

  task->error = ERR_NONE;
}

static inline void do_pause_task(struct int_stack_frame *stack_frame,
                                 int cpu, struct cpu_data *cpud,
                                 struct cpu_task *task) {
  struct thread_data *thrd = task->thread;
  if (thrd->state != THREAD_STATE_RUNNING || thrd->cpu != cpu) {
    task->error = ERR_NOT_FOUND;
    return;
  }

  bool current = cpud->priorities[cpud->current_priority].active_tail == thrd;
  struct cpu_priority *prio = &cpud->priorities[thrd->real_priority];

  if (thrd->next)
    thrd->next->prev = thrd->prev;
  if (thrd->prev)
    thrd->prev->next = thrd->next;

  if (prio->active_tail == thrd) {
    prio->active_tail = thrd->next;
    if (!prio->active_tail) {
      prio->active_tail = prio->expired_tail;
      prio->expired_tail = prio->expired_head = NULL;
    }
  }
  else {
    if (prio->expired_head == thrd)
      prio->expired_head = thrd->prev;
    if (prio->expired_tail == thrd)
      prio->expired_tail = thrd->next;
  }

  if (!--prio->total_threads)
    BIT_ARRAY_RESET(cpud->priority_mask, thrd->real_priority);
  cpud->total_threads--;

  if (current) {
    if (cpud->total_threads) {
      int highest = __find_mask_bsf(cpud->priority_mask, PRIORITY_MASK_SIZE);
      cpud->current_priority = highest;
      cpud->load_context = true;
    }

    __set_thread(NULL); // halt CPU till next timer interrupt
    if (stack_frame) {
      __load_thread_context(stack_frame, &thrd->context);
      __store_halt(stack_frame);
    }
    else // postpone saving context
      task->type = CPU_TASK_SELF_HALT;
  }

  task->error = ERR_NONE;
}

static inline void do_self_halt_task(struct int_stack_frame *stack_frame,
                                     struct cpu_data *cpud,
                                     struct cpu_task *task) {
  __store_int_enabled(stack_frame);
  __load_thread_context(stack_frame, &task->thread->context);
  __store_halt(stack_frame);

  set_outer_spinlock(false);
  release_spinlock_int(&cpud->lock);
  release_spinlock_int(&inactive.lock);
  if (task->lock)
    release_spinlock_int(task->lock);
}

DEFINE_ISR(task, 0) {
  int cpu = get_cpu();
  struct cpu_data *cpud = &cpus[cpu];
  struct cpu_task *task = &cpud->task;

  switch (task->type) {
  case CPU_TASK_RESUME:
    do_resume_task(cpu, cpud, task);
    break;
  case CPU_TASK_PAUSE:
    do_pause_task(stack_frame, cpu, cpud, task);
    break;
  case CPU_TASK_SELF_HALT:
    do_self_halt_task(stack_frame, cpud, task);
    break;
  default:
    task->error = ERR_BAD_INPUT;
  }

  __set_eoi();
}

void init_scheduler(void) {
  __init_scheduler();

  int cpu = get_cpu();
  if (cpu == get_bsp_cpu()) {
    create_spinlock(&all.lock);
    create_spinlock(&inactive.lock);
  }

  create_spinlock(&cpus[cpu].lock);
  LOG_DEBUG("done (CPU: %d)", cpu);
}
