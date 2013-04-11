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

#define CPU_TASK_RESUME 1
#define CPU_TASK_PAUSE 2

struct cpu_task {
  IN uint8_t type : 8;
  OUT uint8_t halt : 1;
  OUT volatile err_code error;
  IN_OUT struct thread_data *thread;
};

#define PRIORITY_MASK_SIZE SIZE_ELEMENTS(CONFIG_SCHEDULER_PRIORITIES, 64)

static struct cpu_data {
  struct spinlock lock;
  int current_priority;
  struct cpu_priority priorities[CONFIG_SCHEDULER_PRIORITIES];
  uint64_t priority_mask[PRIORITY_MASK_SIZE];
  int total_threads;
  bool load_context;
  struct cpu_task task;
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
      return i * 64 + bsf(mask[i]);
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
  thread->state = THREAD_STATE_UNKNOWN;
  thread->run_time = 0;

  acquire_spinlock(&all.lock, 0);
  thread->magic = THREAD_MAGIC;
  thread->all_next = all.tail;
  if (all.tail)
    all.tail->prev = thread;
  all.tail = thread;
  all.total_threads++;
  release_spinlock(&all.lock);

  acquire_spinlock(&inactive.lock, 0);
  thread->state = THREAD_STATE_PAUSED;
  thread->next = inactive.tail;
  if (inactive.tail)
    inactive.tail->prev = thread;
  inactive.tail = thread;
  inactive.total_threads++;
  release_spinlock(&inactive.lock);

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
  int state;

  acquire_spinlock(&inactive.lock, 0);
  if (thrd->state == THREAD_STATE_PAUSED ||
      thrd->state == THREAD_STATE_STOPPED) {
    state = thrd->state, thrd->state = THREAD_STATE_UNKNOWN;
    if (thrd->next)
      thrd->next = thrd->prev;
    if (thrd->prev)
      thrd->prev = thrd->next;
    if (inactive.tail == thrd)
      inactive.tail = thrd->next;
    inactive.total_threads--;
  }
  else
    err = ERR_BAD_STATE;
  release_spinlock(&inactive.lock);

  if (!err) {
    acquire_spinlock(&all.lock, 0);
    if (thrd->all_next)
      thrd->all_next = thrd->all_prev;
    if (thrd->all_prev)
      thrd->all_prev = thrd->all_next;
    if (all.tail == thrd)
      all.tail = thrd->next;
    all.total_threads--;
    thrd->magic = 0;
    release_spinlock(&all.lock);

    thrd->state = state;
    thrd->prev = thrd->next = NULL;
    thrd->all_prev = thrd->all_next = NULL;
    *thread = thrd;
  }

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
      int temp = cpus[cpu].total_threads;
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
    issue_cpu_interrupt(get_cpu_desc(cpu)->apic_id, INT_VECTOR_SCHEDULER_TASK);
    while (cpud->task.error == ERR_BUSY);
  }
  *task = cpud->task;
  release_spinlock(&cpud->lock);
}

err_code resume_thread(thread_id id) {
  err_code err = ERR_NONE;
  CAST_TO_THREAD(thrd, id);

  acquire_spinlock(&inactive.lock, 0);
  if (thrd->state == THREAD_STATE_PAUSED) {
    thrd->state = THREAD_STATE_UNKNOWN;
    if (thrd->next)
      thrd->next->prev = thrd->prev;
    if (thrd->prev)
      thrd->prev->next = thrd->next;
    if (inactive.tail == thrd)
      inactive.tail = thrd->next;
    inactive.total_threads--;
  }
  else
    err = ERR_BAD_STATE;
  release_spinlock(&inactive.lock);

  if (!err) {
    thrd->real_priority = thrd->priority;
    thrd->quantum = 0;
    update_priority_quantum(thrd);

    struct cpu_task task = { .type = CPU_TASK_RESUME, .thread = thrd };
    run_cpu_task(find_least_loaded_cpu(thrd->affinity), &task);
  }

  return err;
}

#define TASK_RETRY_COUNT 3

static err_code deactivate_thread(bool stop, thread_id id, uint64_t output) {
  struct cpu_task task;
  CAST_TO_THREAD(thrd, id);

  for (int i = 0; i < TASK_RETRY_COUNT; i++) {
    int state = thrd->state, cpu = thrd->cpu;
    switch (state) {
    case THREAD_STATE_UNKNOWN:
      break;

    case THREAD_STATE_RUNNING:
      task = (struct cpu_task) { .type = CPU_TASK_PAUSE, .thread = thrd };
      run_cpu_task(cpu, &task);
      if (task.error != ERR_NOT_FOUND) {
        if (!task.error) {
          thrd->prev = NULL;
          acquire_spinlock(&inactive.lock, 0);
          thrd->state = stop ? THREAD_STATE_STOPPED : THREAD_STATE_PAUSED;
          thrd->next = inactive.tail;
          if (inactive.tail)
            inactive.tail->prev = thrd;
          inactive.tail = thrd;
          inactive.total_threads++;
          release_spinlock(&inactive.lock);
          if (task.halt)
            ASMV("jmp halt");
        }
        return task.error;
      }
      break;

    case THREAD_STATE_PAUSED:
      if (stop) {
        err_code err = ERR_NONE;
        acquire_spinlock(&inactive.lock, 0);
        if (thrd->state == THREAD_STATE_PAUSED) {
          thrd->state = THREAD_STATE_STOPPED;
          thrd->output = output;
        }
        else
          err = ERR_NOT_FOUND;
        release_spinlock(&inactive.lock);
        if (!err)
          return ERR_NONE;
        break;
      }
      return ERR_BAD_STATE;

    case THREAD_STATE_STOPPED:
      return ERR_BAD_STATE;
    }
  }

  return ERR_BUSY;
}

err_code pause_thread(thread_id id) {
  err_code err = deactivate_thread(false, id, 0);
  return err;
}

err_code stop_thread(thread_id id, uint64_t output) {
  err_code err = deactivate_thread(true, id, output);
  return err;
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
    kprintf("\nstack overrun (CPU: %d, thread %lX):\n", cpu, (uint64_t)thread);
    dump_int_stack_frame(stack_frame);
    kprintf("\n\n\n\n");
    ASMV("jmp halt");
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

  if (!prio->active_tail) {
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
  set_apic_eoi();
}

static inline void do_resume_task(UNUSED struct int_stack_frame *stack_frame,
                                  int cpu, struct cpu_data *cpud,
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
    thrd->next = thrd->prev;
  if (thrd->prev)
    thrd->prev = thrd->next;

  if (prio->expired_head == thrd)
    prio->expired_head = thrd->prev;
  else if (prio->expired_tail == thrd)
    prio->expired_tail = thrd->next;
  else if (prio->active_tail == thrd) {
    prio->active_tail = thrd->next;
    if (!prio->active_tail) {
      prio->active_tail = prio->expired_tail;
      prio->expired_tail = prio->expired_head = NULL;
    }
  }

  thrd->state = THREAD_STATE_UNKNOWN;
  if (!--prio->total_threads)
    BIT_ARRAY_RESET(cpud->priority_mask, thrd->real_priority);
  cpud->total_threads--;

  if (current) {
    wrmsr(MSR_FS_BASE, 0);
    if (cpud->total_threads) {
      int highest = find_mask_bsf(cpud->priority_mask, PRIORITY_MASK_SIZE);
      cpud->current_priority = highest;
      cpud->load_context = true;
    }

    if (stack_frame) {
      extern int halt;
      stack_frame->rip = (uint64_t)&halt;
    }
    else
      task->halt = true;
  }

  task->error = ERR_NONE;
}

DEFINE_ISR(task, 0) {
  int cpu = get_cpu();
  struct cpu_data *cpud = &cpus[cpu];
  struct cpu_task *task = &cpud->task;

  switch (task->type) {
  case CPU_TASK_RESUME: do_resume_task(stack_frame, cpu, cpud, task); break;
  case CPU_TASK_PAUSE: do_pause_task(stack_frame, cpu, cpud, task); break;
  default: task->error = ERR_BAD_INPUT;
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
  start_apic_timer(CONFIG_SCHEDULER_TIMER_INTERVAL, true);
  LOG_DEBUG("done (CPU: %d)", cpu);
}
