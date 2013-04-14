#include "memory.h"
#include "schedule.h"
#include "sync.h"

INTERNAL struct spinlock *__if_owner = NULL;

static struct mem_pool mutex_node_pool;

struct __mutex_node {
  struct __mutex_node *next;
  thread_id id;
};

INTERNAL err_code __sleep_in_mutex(struct mutex *mutex) {
  struct __mutex_node *node = NULL;
  bool acquired;

  acquire_spinlock(&mutex->ilock, 0);
  acquired = acquire_spinlock(&mutex->mlock, 1);
  if (!acquired) {
    node = alloc_block(&mutex_node_pool);
    if (node) {
      node->next = NULL;
      node->id = get_thread();
      if (mutex->head)
        mutex->head->next = node;
      mutex->head = node;
      if (!mutex->tail)
        mutex->tail = node;
      pause_this_thread(&mutex->ilock);
    }
  }
  if (!node)
    release_spinlock(&mutex->ilock);

  return (acquired || node) ? ERR_NONE : ERR_OUT_OF_MEMORY;
}

INTERNAL void __awake_in_mutex(struct mutex *mutex) {
  struct __mutex_node *node;
  err_code err;

  do {
    node = mutex->tail;
    mutex->tail = node->next;
    if (mutex->head == node)
      mutex->head = NULL;
    err = resume_thread(node->id);
    free_block(&mutex_node_pool, node);
  }
  while (mutex->tail && err);

  if (!mutex->tail)
    release_spinlock(&mutex->mlock);
}

void init_sync(void) {
  if (create_mem_pool(sizeof(struct __mutex_node), &mutex_node_pool)) {
    LOG_ERROR("Failed to create a memory pool");
    ASMV("jmp halt");
  }
}
