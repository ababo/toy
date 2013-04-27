#include "test.h"

#define STACK_SIZE 0x1000
#define DEFAULT_PRIORITY 3

void set_test_thread_affinity(struct thread_data *thread, bool bsp_only) {
  for (int i = 0; i < THREAD_AFFINITY_SIZE; i++)
    thread->affinity[i] = bsp_only ? (i ? 0 : 1) : UINT64_MAX;
}

struct thread_data *create_test_thread(struct mem_pool *pool,
                                       thread_proc proc, uint64_t input) {
  struct thread_data *thrd = alloc_block(pool);
  if (!thrd) {
    LOG_ERROR("Failed to allocate memory");
    return NULL;
  }

  memset(thrd, 0, sizeof(*thrd));
  thrd->stack = (uint8_t*)(thrd + 1);
  thrd->stack_size = STACK_SIZE;
  set_test_thread_affinity(thrd, false);
  thrd->priority = DEFAULT_PRIORITY;
  thrd->fixed_priority = true;
  set_thread_context(thrd, proc, input);

  return thrd;
}

void destroy_test_thread(struct mem_pool *pool, struct thread_data *thread) {
  free_block(pool, thread);
}

DEFINE_TEST(all) {
  BEGIN_TEST();
  struct mem_pool pool;
  if (create_mem_pool(sizeof(struct thread_data) + STACK_SIZE, &pool)) {
    LOG_ERROR("Failed to create a memory pool");
    return false;
  }

  //  ADD_TEST(scheduler, &pool);
  ADD_TEST(sync, &pool);

  destroy_mem_pool(&pool);
  END_TEST();
}
