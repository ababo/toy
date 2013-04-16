#include "../sync.h"
#include "test.h"

#define WAIT_ITERATIONS 10000000

static struct mutex mutex;

static uint64_t mutex_proc(UNUSED uint64_t input) {
  LOG_DEBUG("started");
  return 0;
}

DEFINE_SUBTEST(mutex, struct mem_pool *thread_pool) {
  BEGIN_TEST();

  struct thread_data *thrd1, *thrd2;
  create_mutex(&mutex);

  thrd1 = create_test_thread(thread_pool, mutex_proc, 0);
  thrd2 = create_test_thread(thread_pool, mutex_proc, 0);
  if (!thrd1 || !thrd2)
    return false;

  thread_id id1, id2;
  attach_thread(thrd1, &id1);
  attach_thread(thrd2, &id2);

  resume_thread(id1);
  resume_thread(id2);

  ADD_TEST_CASE("just a stub", true);

  END_TEST();
}

DEFINE_TEST(sync, struct mem_pool *thread_pool) {
  BEGIN_TEST();
  ADD_TEST(mutex, thread_pool);
  END_TEST();
}
