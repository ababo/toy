#include "apic.h"
#include "cpu_info.h"
#include "mem_mgr.h"
#include "interrupt.h"
#include "schedule.h"
#include "sync.h"

#define THREAD_DESC_MAGIC 0x3141592653589793

struct task {
  uint32_t type;
  uint32_t result;
  union {
    struct thread_desc desc;
  } u;
};

static struct {
  struct spinlock lock;
  struct {
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

static void idle_proc(UNUSED uint64_t data) {

}

static void create_idle_thread(void) {
  struct thread_desc *idle = kmalloc(sizeof(*idle));
  if (!idle) {
    LOG_ERROR("failed allocate memory");
    return;
  }

  *idle = (struct thread_desc) { .magic = THREAD_DESC_MAGIC, .idle = true };
  

}

void init_scheduler(void) {
  int cpui = get_cpu_index();
  if (cpui == get_bsp_cpu_index()) {
    set_isr(INT_VECTOR_APIC_TIMER, timer_isr_getter());
    set_isr(INT_VECTOR_SCHEDULER_TASK, task_isr_getter());
  }

  create_idle_thread();
  start_apic_timer(CONFIG_SCHEDULER_TIMER_INTERVAL, true);

}
