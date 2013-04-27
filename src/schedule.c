#include "apic.h"
#include "boot.h"
#include "cpu_info.h"
#include "schedule.h"
#include "sync.h"

#define THREAD_MAGIC 0x3141592653589793
#define STACK_OVERRUN_MAGIC 0x3979853562951413

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
} cpus[CONFIG_CPUS_MAX];

static struct thread_list {
  struct spinlock lock;
  int total_threads;
  struct thread_data *tail;
} all, inactive;

thread_id get_thread(void) {
  return rdmsr(MSR_FS_BASE);
}

ASM(".global exit_thread\nexit_thread:\n"
    "pushq %rax\ncallq get_thread\n"
    "movq %rax, %rdi\npopq %rsi\n"
    "callq stop_thread");

err_code set_thread_context(struct thread_data *thread, thread_proc proc,
                            uint64_t input) {
  if (!thread->stack || thread->stack_size < THREAD_STACK_SIZE_MIN)
    return ERR_BAD_INPUT;

  extern int exit_thread;
  *(uint64_t*)thread->stack = STACK_OVERRUN_MAGIC;
  uint64_t *top = (uint64_t*)(thread->stack + thread->stack_size - 8);
  *top = (uint64_t)&exit_thread;

  thread->context.rflags = RFLAGS_IF;
  thread->context.cs = SEGMENT_CODE;
  thread->context.ss = SEGMENT_DATA;
  thread->context.rsp = (uint64_t)top;
  thread->context.rip = (uint64_t)proc;
  thread->context.rdi = input;
  return ERR_NONE;
}

static inline int find_mask_bsf(const uint64_t *mask, int size) {
  for (int i = 0; i < size; i++)
    if (mask[i])
      return i * 64 + bsfq(mask[i]);
  return -1;
}

err_code attach_thread(struct thread_data *thread, thread_id *id) {
  if (thread->magic == THREAD_MAGIC) // already attached
    return ERR_BAD_STATE;

  if (!thread->stack || thread->stack_size < THREAD_STACK_SIZE_MIN ||
      *(uint64_t*)thread->stack != STACK_OVERRUN_MAGIC ||
      thread->priority >= CONFIG_SCHEDULER_PRIORITIES)
    return ERR_BAD_INPUT;

  int bsf = find_mask_bsf(thread->affinity, THREAD_AFFINITY_SIZE);
  if (bsf == -1 || bsf >= get_started_cpus())
    return ERR_BAD_INPUT;

  thread->prev = thread->all_prev = NULL;
  thread->state = THREAD_STATE_PAUSED;
  thread->run_time = 0;

  acquire_spinlock(&all.lock, 0);
  acquire_spinlock(&inactive.lock, 0);

  thread->magic = THREAD_MAGIC;
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

  *id = (thread_id)thread;
  return ERR_NONE;
}

#define CAST_TO_THREAD(thrd, id)                        \
  struct thread_data *thrd = (struct thread_data*)id;   \
  if (thrd->magic != THREAD_MAGIC)                      \
    return ERR_NOT_FOUND;

err_code detach_thread(thread_id id, struct thread_data **thread) {
  err_code err = ERR_NONE;
  CAST_TO_THREAD(thrd, id);

  acquire_spinlock(&all.lock, 0);
  acquire_spinlock(&inactive.lock, 0);

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
  for (int i = 0; i < get_started_cpus(); i++)
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
    handle_task_int(NULL, 0);
  else {
    cpud->task.error = ERR_BUSY;
    issue_cpu_interrupt(cpu, INT_VECTOR_SCHEDULER_TASK);
    while (cpud->task.error == ERR_BUSY) { }
  }
  *task = cpud->task;
  if (task->type != CPU_TASK_SELF_HALT)
    release_spinlock(&cpud->lock);
}

err_code resume_thread(thread_id id) {
  err_code err = ERR_NONE;
  CAST_TO_THREAD(thrd, id);

  acquire_spinlock(&inactive.lock, 0);
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
    ASMV("int %0" : : "i"(INT_VECTOR_SCHEDULER_TASK));
    *self_halt = true;
  }

  return ERR_NONE;
}

#define TASK_RETRY_COUNT 3

static err_code deactivate_thread(bool stop, thread_id id, uint64_t output,
                                  struct spinlock *lock_to_release) {
  err_code err;
  bool self_halt = false;
  CAST_TO_THREAD(thrd, id);

  acquire_spinlock(&inactive.lock, 0);

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

  if (!self_halt)
    release_spinlock(&inactive.lock);

  return (err == ERR_NOT_FOUND) ? ERR_BUSY : err;
}

err_code pause_thread(thread_id id) {
  return deactivate_thread(false, id, 0, NULL);
}

err_code pause_this_thread(struct spinlock *lock_to_release) {
  return deactivate_thread(false, get_thread(), 0, lock_to_release);
}

err_code stop_thread(thread_id id, uint64_t output) {
  return deactivate_thread(true, id, output, NULL);
}

static uint64_t ticks, timer_ticks[CONFIG_CPUS_MAX];
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

static inline void check_stack_overrun(struct int_stack_frame *stack_frame,
                                       int cpu, struct thread_data *thread) {
  if (*(volatile uint64_t*)thread->stack != STACK_OVERRUN_MAGIC) {
    ASMV("cli");
    // make sure calling release_spinlock will not enable interrupts
    extern struct spinlock *__outer_spinlocks[];
    __outer_spinlocks[cpu] = (struct spinlock*)1;

    kprintf("\nstack overrun (CPU: %d, thread: %lX):\n",
            cpu, (uint64_t)thread);
    dump_int_stack_frame(stack_frame);

    ASMV("jmp halt");
  }
}

static inline void handle_timer(int cpu) {
  if (cpu == get_bsp_cpu())
    ticks++;

  if (timer_ticks[cpu] && timer_ticks[cpu] <= ticks) {
    extern struct spinlock *__outer_spinlocks[];
    uint64_t prev_ticks = timer_ticks[cpu];
    timer_ticks[cpu] = 0;

    // make sure calling release_spinlock will not enable interrupts
    __outer_spinlocks[cpu] = (struct spinlock*)1;
    timer_proc_(prev_ticks);
    __outer_spinlocks[cpu] = NULL;
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
    *stack_frame = thrd->context;
    wrmsr(MSR_FS_BASE, (uint64_t)thrd);
    cpud->load_context = false;
    goto exit;
  }

  check_stack_overrun(stack_frame, cpu, thrd);
  thrd->run_time++;

  int highest = find_mask_bsf(cpud->priority_mask, PRIORITY_MASK_SIZE);
  if (highest >= cpud->current_priority && thrd->quantum) {
    thrd->quantum--;
    goto exit;
  }

  prio->active_tail = thrd->next;
  if (!--prio->total_threads)
    BIT_ARRAY_RESET(cpud->priority_mask, cpud->current_priority);

  thrd->context = *stack_frame;
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
    *stack_frame = thrd2->context;
    wrmsr(MSR_FS_BASE, (uint64_t)thrd2);
  }

exit:
  handle_timer(cpu);
  set_apic_eoi();
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
      int highest = find_mask_bsf(cpud->priority_mask, PRIORITY_MASK_SIZE);
      cpud->current_priority = highest;
      cpud->load_context = true;
    }

    wrmsr(MSR_FS_BASE, 0); // halt CPU till next timer interrupt
    if (stack_frame) {
      extern int halt;
      thrd->context = *stack_frame;
      stack_frame->rip = (uint64_t)&halt;
    }
    else // postpone saving context
      task->type = CPU_TASK_SELF_HALT;
  }

  task->error = ERR_NONE;
}

static inline void do_self_halt_task(struct int_stack_frame *stack_frame,
                                     int cpu, struct cpu_data *cpud,
                                     struct cpu_task *task) {
  extern int halt;
  stack_frame->rflags |= RFLAGS_IF;
  task->thread->context = *stack_frame;
  stack_frame->rip = (uint64_t)&halt;

  extern struct spinlock *__outer_spinlocks[];
  __outer_spinlocks[cpu] = NULL;
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
    do_self_halt_task(stack_frame, cpu, cpud, task);
    break;
  default:
    task->error = ERR_BAD_INPUT;
  }

  set_apic_eoi();
}

void init_scheduler(void) {
  int cpu = get_cpu();
  if (cpu == get_bsp_cpu()) {
    set_isr(INT_VECTOR_APIC_TIMER, get_timer_isr());
    set_isr(INT_VECTOR_SCHEDULER_TASK, get_task_isr());
    create_spinlock(&all.lock);
    create_spinlock(&inactive.lock);
  }

  create_spinlock(&cpus[cpu].lock);
  start_apic_timer(CONFIG_SCHEDULER_TICK_INTERVAL, true);
  LOG_DEBUG("done (CPU: %d)", cpu);
}
