#include "apic.h"
#include "boot.h"
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

static struct cpu_data {
  struct spinlock lock;
  int current_priority;
  struct priority_data {
    struct thread_desc *expired_head, *expired_tail, *active_tail;
    int total;
  } priorities[CONFIG_SCHEDULER_PRIORITIES];
  // fast lookup bitmask: 1 if corresponding priority has threads
  uint64_t priority_mask[SIZE_ELEMENTS(CONFIG_SCHEDULER_PRIORITIES, 64)];
  struct task *task;
} cpus[CONFIG_CPUS_MAX];

static inline int get_highest_priority(const uint64_t *priority_mask) {
  for (int i = 0; ; i++)
    if (priority_mask[i])
      return i * 64 + bsf(priority_mask[i]);
}

static inline void update_priority_quantum(struct thread_desc *desc) {
  /// TODO: make priority and quantum change dynamically
  desc->real_priority = desc->priority;
  desc->quantum = 100;
}

static inline void load_context(const struct thread_context *context,
                                struct int_stack_frame *frame) {
  /// TODO: optimize for speed
  memcpy((uint8_t*)frame + sizeof(*frame) - sizeof(*context),
         context, sizeof(*context));
}

static inline void store_context(const struct int_stack_frame *frame,
                                 struct thread_context *context) {
  /// TODO: optimize for speed
  memcpy(context, (uint8_t*)frame + sizeof(*frame) - sizeof(*context),
         sizeof(*context));
}

ISR_DEFINE(timer, 0) {
  struct cpu_data *cpud = &cpus[get_cpu()];
  struct priority_data *priod = &cpud->priorities[cpud->current_priority];
  struct thread_desc *thrd = priod->active_tail;

  int highest_prio = get_highest_priority(cpud->priority_mask);
  if (highest_prio < cpud->current_priority || !thrd->quantum) {
    update_priority_quantum(thrd);
    if (thrd->skip_store_context)
      thrd->skip_store_context = false;
    else
      store_context(stack_frame, &thrd->context);

    struct priority_data *priod2 = &cpud->priorities[thrd->real_priority];
    BIT_ARRAY_SET(cpud->priority_mask, thrd->real_priority);
    priod2->total++;
    if (priod2->expired_tail)
      priod2->expired_head->next = thrd, priod2->expired_head = thrd;
    else
      priod2->expired_head = priod2->expired_tail = thrd;

    priod->active_tail = thrd->next;
    if (!priod->active_tail) {
      priod->active_tail = priod->expired_tail;
      priod->expired_tail = priod->expired_head = NULL;
    }
    if (!--priod->total)
      BIT_ARRAY_RESET(cpud->priority_mask, cpud->current_priority);

    cpud->current_priority = get_highest_priority(cpud->priority_mask);
    thrd = cpud->priorities[cpud->current_priority].active_tail;
    load_context(&thrd->context, stack_frame);
  }
  else
    thrd->quantum--;

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
    *t = (struct thread_desc) {
      .magic = THREAD_DESC_MAGIC,
      .context = { .frame = { .cs = SEGMENT_CODE, .ss = SEGMENT_DATA } }
    };

    t->stack_size = stack_size;
    t->stack = (uint8_t*)(t + 1);
    *(uint64_t*)t->stack = STACK_OVERRUN_MAGIC;

    extern int exit_thread;
    uint64_t *top = (uint64_t*)(t->stack + t->stack_size - 8);
    *top = (uint64_t)&exit_thread, t->context.frame.rsp = (uint64_t)top;

    t->context.frame.rip = (uint64_t)proc;
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
  int prio = CONFIG_SCHEDULER_PRIORITIES - 1;

  BIT_ARRAY_SET(idle->affinity, cpu);
  idle->priority = idle->real_priority = prio;
  idle->skip_store_context = idle->fixed_priority =
    idle->fixed_affinity = idle->protected = true;

  struct cpu_data *cpud = &cpus[cpu];
  struct priority_data *priod = &cpud->priorities[prio];
  priod->active_tail = idle, priod->total = 1;
  BIT_ARRAY_SET(cpud->priority_mask, prio);
  cpud->current_priority = prio;
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
