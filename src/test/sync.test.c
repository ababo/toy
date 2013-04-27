#include "../sync.h"
#include "test.h"

#define WAIT_ITERATIONS 100000000

static struct mutex mutex;
static volatile bool acquired[2], release;

static uint64_t mutex_proc(uint64_t input) {
  LOG_DEBUG("acquiring...");
  acquire_mutex(&mutex);
  LOG_DEBUG("acquired");
  acquired[input] = true;
  while (!release) { }
  LOG_DEBUG("releasing...");
  release_mutex(&mutex);
  LOG_DEBUG("released");
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
       !acquired[0] && !acquired[1] && i < WAIT_ITERATIONS; i++) { }
  ADD_TEST_CASE("make sure no one acquired", !acquired[0] && !acquired[1]);

  release_mutex(&mutex);
  for (volatile int i = 0;
       !(acquired[0] && acquired[1]) && i < WAIT_ITERATIONS; i++) { }
  ADD_TEST_CASE("make sure only one acquired", acquired[0] ^ acquired[1]);

  release = true;
  for (volatile int i = 0;
       (!acquired[0] || !acquired[1]) && i < WAIT_ITERATIONS; i++) { }
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

  if (tolerate_errors || passed) {
    for (volatile int i = 0; i < WAIT_ITERATIONS; i++) { }
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

  for (volatile int i = 0; i < WAIT_ITERATIONS; i++) { }
  detach_thread(id1, &thrd1);
  detach_thread(id2, &thrd2);
  destroy_test_thread(thread_pool, thrd1);
  destroy_test_thread(thread_pool, thrd2);

  END_TEST();
}

static uint64_t sleep_proc(uint64_t input) {
  LOG_DEBUG("falling asleep for %d seconds... (thread: %lX)",
            (int)input, get_thread());
  uint64_t period = input * 1000000;
  uint64_t before = get_ticks();
  sleep(period);
  uint64_t after = get_ticks();
  int64_t diff = (int64_t)after - (int64_t)before -
    period / CONFIG_SCHEDULER_TICK_INTERVAL;
  LOG_DEBUG("awaken with ticks diff: %ld (thread: %lX)", diff, get_thread());
  return ABS(diff) <= 20;
}

DEFINE_SUBTEST(sleep, struct mem_pool *thread_pool) {
  BEGIN_TEST();

  thread_id id1, id2, id3;
  struct thread_data *thrd1, *thrd2, *thrd3;
  thrd1 = create_test_thread(thread_pool, sleep_proc, 2);
  thrd2 = create_test_thread(thread_pool, sleep_proc, 3);
  thrd3 = create_test_thread(thread_pool, sleep_proc, 1);
  if (!thrd1 || !thrd2 || !thrd3)
    return false;
  set_test_thread_affinity(thrd1, true);
  set_test_thread_affinity(thrd2, true);
  set_test_thread_affinity(thrd3, true);
  attach_thread(thrd1, &id1);
  attach_thread(thrd2, &id2);
  attach_thread(thrd3, &id3);
  resume_thread(id1);
  resume_thread(id2);
  resume_thread(id3);

  volatile struct thread_data *temp;
  for (temp = thrd3; temp->state != THREAD_STATE_STOPPED; ) { }
  ADD_TEST_CASE("first awaken in time", temp->output);
  if (tolerate_errors || passed)
    for (temp = thrd1; temp->state != THREAD_STATE_STOPPED; ) { }
  ADD_TEST_CASE("second awaken in time", temp->output);
  if (tolerate_errors || passed)
    for (temp = thrd2; temp->state != THREAD_STATE_STOPPED; ) { }
  ADD_TEST_CASE("third awaken in time", temp->output);

  detach_thread(id1, &thrd1);
  detach_thread(id2, &thrd2);
  detach_thread(id3, &thrd3);
  destroy_test_thread(thread_pool, thrd1);
  destroy_test_thread(thread_pool, thrd2);
  destroy_test_thread(thread_pool, thrd3);

  END_TEST();
}

DEFINE_TEST(sync, struct mem_pool *thread_pool) {
  BEGIN_TEST();
  ADD_TEST(mutex, thread_pool);
  ADD_TEST(sleep, thread_pool);
  END_TEST();
}
