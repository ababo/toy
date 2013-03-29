#include "apic.h"
#include "cpu_info.h"
#include "mem_mgr.h"
#include "interrupt.h"
#include "schedule.h"
#include "sync.h"

#define THREAD_DESC_MAGIC 0x3141592653589793
#define STACK_OVERRUN_MAGIC 0x3979853562951413

struct task {
  uint32_t type;
  uint32_t result;
  union {
    struct thread_desc desc;
  } u;
};

static struct {
  struct spinlock lock;
  struct priority {
    struct thread_desc *expired_head, *expired_tail, *active_tail;
    uint32_t total;
  } priorities[CONFIG_SCHEDULER_PRIORITIES];
  // fast lookup bitmask: 1 if an appropriate priority has threads 
  uint64_t priority_mask[SIZE_ELEMENTS(CONFIG_SCHEDULER_PRIORITIES, 64)];
  struct task *task;
} cpus[CONFIG_CPUS_MAX];

ISR_DEFINE(timer, 0) {


  set_apic_eoi();
}

ISR_DEFINE(task, 0) {

}

ASM(".global exit_thread\nexit_thread:\n"
    "callq get_thread\nmovq %rax, %rdi\n"
    "callq destroy_thread");

static struct thread_desc *new_thread(thread_proc proc, uint64_t data,
                                      size_t stack_size) {
  struct thread_desc *t = kmalloc(sizeof(*t) + 8 + stack_size);
  if (t) {
    *t = (struct thread_desc) { .magic = THREAD_DESC_MAGIC };

    t->stack_size = stack_size;
    t->stack = (uint8_t*)(t + 1);
    *(uint64_t*)t->stack = STACK_OVERRUN_MAGIC;

    extern int exit_thread;
    uint64_t *top = (uint64_t*)(t->stack + t->stack_size);
    t->context.rsp = t->context.rbp = (uint64_t)top;
    *--top = (uint64_t)&exit_thread;

    t->context.rip = (uint64_t)proc;
    t->context.rdi = data;
  }
  return t;
}

static void idle_proc(UNUSED uint64_t data) {
  for (;;) {
    kprintf("Hello from idle thread (CPU: %d)\n", get_cpu());
    for (volatile int i = 0; i < 1000000000; i++);
  }
}

static void create_idle_thread(void) {
  struct thread_desc *idle = new_thread(idle_proc, 0, CONFIG_IDLE_STACK_SIZE);
  if (!idle) {
    LOG_ERROR("failed allocate memory");
    return;
  }

  int cpu = get_cpu();
  BIT_ARRAY_SET(idle->affinity, cpu);
  idle->fixed_priority = true;

  int prio = CONFIG_SCHEDULER_PRIORITIES - 1;
  struct priority *p = &cpus[cpu].priorities[prio];
  p->active_tail = idle, p->total = 1;
  BIT_ARRAY_SET(cpus[cpu].priority_mask, prio);
}

void init_scheduler(void) {
  int cpui = get_cpu();
  if (cpui == get_bsp_cpu()) {
    set_isr(INT_VECTOR_APIC_TIMER, timer_isr_getter());
    set_isr(INT_VECTOR_SCHEDULER_TASK, task_isr_getter());
  }

  create_idle_thread();
  start_apic_timer(CONFIG_SCHEDULER_TIMER_INTERVAL, true);
}

error destroy_thread(UNUSED thread_id id) {

  return ERR_NONE;
}

thread_id get_thread(void) {

  return ERR_NONE;
}
