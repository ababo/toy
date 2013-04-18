#include "../sync.h"
#include "test.h"

#define WAIT_ITERATIONS 10000000

static struct mutex mutex;
static volatile bool acquired[2], release;

static uint64_t mutex_proc(UNUSED uint64_t input) {
  acquire_mutex(&mutex);
  acquired[input] = true;
  while (!release);
  release_mutex(&mutex);
  return 0;
}

DEFINE_SUBTEST(mutex_wait, thread_id id1, thread_id id2) {
  BEGIN_TEST();

  acquired[0] = acquired[1] = release = false;
  create_mutex(&mutex);
  acquire_mutex(&mutex);
  resume_thread(id1);
  resume_thread(id2);
  for (volatile int i = 0;
       !acquired[0] && !acquired[1] && i < WAIT_ITERATIONS; i++);
  ADD_TEST_CASE("make sure no one acquired", !acquired[0] && !acquired[1]);

  release_mutex(&mutex);
  for (volatile int i = 0;
       (!acquired[0] || !acquired[1]) && i < WAIT_ITERATIONS; i++);
  ADD_TEST_CASE("make sure only one acquired", acquired[0] ^ acquired[1]);

  release = true;
  for (volatile int i = 0;
       (!acquired[0] || !acquired[1]) && i < WAIT_ITERATIONS; i++);
  ADD_TEST_CASE("make sure both acquired", acquired[0] && acquired[1]);

  END_TEST();
}

DEFINE_SUBTEST(mutex, struct mem_pool *thread_pool) {
  BEGIN_TEST();

  thread_id id1, id2;
  struct thread_data *thrd1, *thrd2;
  thrd1 = create_test_thread(thread_pool, mutex_proc, 0);
  thrd2 = create_test_thread(thread_pool, mutex_proc, 1);
  if (!thrd1 || !thrd2)
    return false;
  attach_thread(thrd1, &id1);
  attach_thread(thrd2, &id2);

  ADD_TEST(mutex_wait, id1, id2);

  if (passed) {
    for (volatile int i = 0; i < WAIT_ITERATIONS; i++);
    LOG_INFO("running on BSP CPU only...");
    detach_thread(id1, &thrd1);
    detach_thread(id2, &thrd2);
    set_thread_context(thrd1, mutex_proc, 0);
    set_thread_context(thrd2, mutex_proc, 1);
    set_test_thread_affinity(thrd1, true);
    set_test_thread_affinity(thrd2, true);
    attach_thread(thrd1, &id1);
    attach_thread(thrd2, &id2);
    ADD_TEST(mutex_wait, id1, id2);
  }

  for (volatile int i = 0; i < WAIT_ITERATIONS; i++);
  detach_thread(id1, &thrd1);
  detach_thread(id2, &thrd2);
  destroy_test_thread(thread_pool, thrd1);
  destroy_test_thread(thread_pool, thrd2);

  END_TEST();
}

DEFINE_TEST(sync, struct mem_pool *thread_pool) {
  BEGIN_TEST();
  ADD_TEST(mutex, thread_pool);
  END_TEST();
}
