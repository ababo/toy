#include "apic.h"
#include "boot.h"
#include "config.h"
#include "cpu_info.h"
#include "interrupt.h"
#include "schedule.h"
#include "sync.h"

#define THREAD_MAGIC 0x3141592653589793
#define STACK_OVERRUN_MAGIC 0x3979853562951413

struct cpu_priority {
  struct thread_data *expired_head, *expired_tail, *active_tail;
  int total_threads;
};

#define CPU_TASK_RESUME 1

struct cpu_task {
  int type;
  volatile err_code error;
  struct thread_data *thread;
};

#define PRIORITY_MASK_SIZE SIZE_ELEMENTS(CONFIG_SCHEDULER_PRIORITIES, 64)

static struct cpu_data {
  struct spinlock lock;
  int current_priority;
  struct cpu_priority priorities[CONFIG_SCHEDULER_PRIORITIES];
  uint64_t priority_mask[PRIORITY_MASK_SIZE];
  int total_threads;
  struct cpu_task task;
} cpus[CONFIG_CPUS_MAX];

static struct thread_list {
  struct spinlock lock;
  int total_threads;
  struct thread_data *tail;
} all, paused, stopped;

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
  thread->context.frame.rsp = (uint64_t)top;
  thread->context.frame.rip = (uint64_t)proc;
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
  if (!thread->stack || thread->stack_size < THREAD_STACK_SIZE_MIN ||
      *(uint64_t*)thread->stack != STACK_OVERRUN_MAGIC ||
      thread->priority >= CONFIG_SCHEDULER_PRIORITIES)
    return ERR_BAD_INPUT;

  int bsf = find_mask_bsf(thread->affinity, THREAD_AFFINITY_SIZE);
  if (bsf == -1 || bsf >= get_started_cpus())
    return ERR_BAD_INPUT;

  thread->magic = THREAD_MAGIC;
  thread->prev = NULL;
  thread->all_prev = NULL;
  thread->run_time = 0;
  thread->state = THREAD_STATE_UNKNOWN;

  acquire_spinlock(&all.lock);
  thread->all_next = all.tail;
  if (all.tail)
    all.tail->prev = thread;
  all.tail = thread;
  all.total_threads++;
  release_spinlock(&all.lock);

  acquire_spinlock(&paused.lock);
  thread->state = THREAD_STATE_PAUSED;
  thread->next = paused.tail;
  if (paused.tail)
    paused.tail->prev = thread;
  paused.tail = thread;
  paused.total_threads++;
  release_spinlock(&paused.lock);

  *id = (thread_id)thread;
  return ERR_NONE;
}

static inline void update_priority_quantum(struct thread_data *thread) {
  /// TODO: make priority and quantum change dynamically
  thread->real_priority = thread->priority;
  thread->quantum = 100;
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

static void run_resume_task(int cpu, struct thread_data *thread) {
  struct cpu_data *cpud = &cpus[cpu];
  acquire_spinlock(&cpud->lock);
  cpud->task.type = CPU_TASK_RESUME;
  cpud->task.error = ERR_BUSY;
  cpud->task.thread = thread;
  issue_cpu_interrupt(get_cpu_desc(cpu)->apic_id, INT_VECTOR_SCHEDULER_TASK);
  while (cpud->task.error == ERR_BUSY);
  release_spinlock(&cpud->lock);
}

#define CAST_TO_THREAD(thrd, id)                        \
  struct thread_data *thrd = (struct thread_data*)id;   \
  if (thrd->magic != THREAD_MAGIC)                      \
    return ERR_BAD_INPUT;

err_code resume_thread(thread_id id) {
  err_code err = ERR_NONE;
  CAST_TO_THREAD(thrd, id);

  acquire_spinlock(&paused.lock);
  if (thrd->state == THREAD_STATE_PAUSED) {
    thrd->state = THREAD_STATE_UNKNOWN;
    if (paused.tail == thrd)
      paused.tail = thrd->next;
    if (thrd->next)
      thrd->next->prev = thrd->prev;
    if (thrd->prev)
      thrd->prev->next = thrd->next;
    paused.total_threads--;
  }
  release_spinlock(&paused.lock);

  if (!err) {
    thrd->real_priority = thrd->priority;
    thrd->quantum = 0;
    update_priority_quantum(thrd);
    run_resume_task(find_least_loaded_cpu(thrd->affinity), thrd);
  }

  return err;
}

thread_id get_thread(void) {
  struct thread_data *thrd;
  ASMV("cli");
  struct cpu_data *cpud = &cpus[get_cpu()];
  thrd = cpud->priorities[cpud->current_priority].active_tail;
  ASMV("sti");
  return (thread_id)thrd;
}

err_code stop_thread(thread_id id, uint64_t output) {
  // TODO: replace this dummy code with a real one
  LOG_DEBUG("thread %X stopped with output %d",
            (uint32_t)id, (uint32_t)output);
  for(;;);
}

ISR_DEFINE(timer, 0) {

  set_apic_eoi();
}

static inline void do_resume_task(struct cpu_data *cpud,
                                  struct cpu_task *task) {
  struct cpu_priority *prio = &cpud->priorities[task->thread->real_priority];
  task->thread->prev = task->thread->next = NULL;
  task->thread->state = THREAD_STATE_RUNNING;
  if (prio->expired_head) {
    prio->expired_head->next = task->thread;
    task->thread->prev = prio->expired_head;
  }
  prio->expired_head = task->thread;
  BIT_ARRAY_SET(cpud->priority_mask, task->thread->real_priority);
  prio->total_threads++;
  task->error = ERR_NONE;
}

ISR_DEFINE(task, 0) {
  struct cpu_data *cpud = &cpus[get_cpu()];
  struct cpu_task *task = &cpud->task;

  switch (task->type) {
  case CPU_TASK_RESUME: do_resume_task(cpud, task); break;
  }
}

void init_scheduler(void) {
  int cpui = get_cpu();
  if (cpui == get_bsp_cpu()) {
    set_isr(INT_VECTOR_APIC_TIMER, timer_isr_getter());
    set_isr(INT_VECTOR_SCHEDULER_TASK, task_isr_getter());
    create_spinlock(&all.lock);
    create_spinlock(&paused.lock);
    create_spinlock(&stopped.lock);
  }

  create_spinlock(&cpus[cpui].lock);
  start_apic_timer(CONFIG_SCHEDULER_TIMER_INTERVAL, true);
  LOG_DEBUG("done (CPU: %d)", cpui);
}
