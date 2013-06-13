#include "../schedule.h"
#include "../memory.h"
#include "test.h"

#define WAIT_ITERATIONS 100000000

static uint64_t counter_proc(uint64_t input) {
  LOG_DEBUG("started");
  volatile uint64_t *counter = (volatile uint64_t*)input;
  for (; *counter; --*counter) { }
  LOG_DEBUG("exiting...");
  return 0;
}

DEFINE_SUBTEST(attach_thread, struct thread_data *thread, thread_id *id) {
  BEGIN_TEST();

  uint64_t temp = thread->stack_size;
  thread->stack_size = THREAD_STACK_SIZE_MIN - 1;
  ADD_TEST_CASE("bad stack size", attach_thread(thread, id) == ERR_BAD_INPUT);
  thread->stack_size = temp;

  temp = (uint64_t)thread->stack;
  thread->stack = NULL;
  ADD_TEST_CASE("bad stack ptr", attach_thread(thread, id) == ERR_BAD_INPUT);
  thread->stack = (uint8_t*)temp;

  temp = *(uint64_t*)thread->stack;
  *(uint64_t*)thread->stack = 0;
  ADD_TEST_CASE("bad stack overrun magic",
                attach_thread(thread, id) == ERR_BAD_INPUT);
  *(uint64_t*)thread->stack = temp;

  temp = thread->priority;
  thread->priority = CONFIG_SCHEDULER_PRIORITIES;
  ADD_TEST_CASE("bad priority", attach_thread(thread, id) == ERR_BAD_INPUT);
  thread->priority = (uint8_t)temp;

  uint64_t affinity[THREAD_AFFINITY_SIZE];
  memcpy(affinity, thread->affinity, THREAD_AFFINITY_SIZE * 8);
  for (int i = 0; i < THREAD_AFFINITY_SIZE; i++)
    thread->affinity[0] = 0;
  ADD_TEST_CASE("bad affinity", attach_thread(thread, id) == ERR_BAD_INPUT);
  memcpy(thread->affinity, affinity, THREAD_AFFINITY_SIZE * 8);

  ADD_TEST_CASE("attaching right", !attach_thread(thread, id));

  ADD_TEST_CASE("attaching already attached",
                attach_thread(thread, id) == ERR_BAD_STATE);

  END_TEST();
}

DEFINE_SUBTEST(resume_thread, thread_id id, volatile uint64_t *counter) {
  BEGIN_TEST();

  ADD_TEST_CASE("bad thread id", resume_thread(1234) == ERR_NOT_FOUND);

  ADD_TEST_CASE("resuming right", !resume_thread(id));

  uint64_t prev_counter = *counter;
  for (volatile int i = 0;
       prev_counter == *counter && i < WAIT_ITERATIONS; i++) { }
  ADD_TEST_CASE("making sure it runs", prev_counter != *counter);

  ADD_TEST_CASE("resuming of already resumed",
                resume_thread(id) == ERR_BAD_STATE);

  END_TEST();
}

DEFINE_SUBTEST(pause_thread, thread_id id, volatile uint64_t *counter) {
  BEGIN_TEST();

  ADD_TEST_CASE("bad thread id", pause_thread(1234) == ERR_NOT_FOUND);

  ADD_TEST_CASE("pausing right", !pause_thread(id));

  uint64_t prev_counter = *counter;
  for (volatile int i = 0;
       prev_counter == *counter && i < WAIT_ITERATIONS; i++) { }
  ADD_TEST_CASE("making sure it paused", prev_counter == *counter);

  ADD_TEST_CASE("pausing paused", pause_thread(id) == ERR_BAD_STATE);

  ADD_TEST_CASE("resuming paused", !resume_thread(id));

  prev_counter = *counter;
  for (volatile int i = 0;
       prev_counter == *counter && i < WAIT_ITERATIONS; i++) { }
  ADD_TEST_CASE("making sure it runs", prev_counter != *counter);

  END_TEST();
}

DEFINE_SUBTEST(detach_thread, thread_id *id, volatile uint64_t *counter) {
  BEGIN_TEST();

  struct thread_data *thrd;
  ADD_TEST_CASE("detaching running",
                detach_thread(1234, &thrd) == ERR_NOT_FOUND);

  ADD_TEST_CASE("detaching running",
                detach_thread(*id, &thrd) == ERR_BAD_STATE);

  ADD_TEST_CASE("pausing and detaching right",
                !pause_thread(*id) && !detach_thread(*id, &thrd) &&
                thrd->state == THREAD_STATE_PAUSED);

  ADD_TEST_CASE("attaching and resuming",
                !attach_thread(thrd, id) && !resume_thread(*id));

  uint64_t prev_counter = *counter;
  for (volatile int i = 0;
       prev_counter == *counter && i < WAIT_ITERATIONS; i++);
  ADD_TEST_CASE("making sure it runs", prev_counter != *counter);

  END_TEST();
}

DEFINE_SUBTEST(stop_thread, thread_id id, volatile uint64_t *counter) {
  BEGIN_TEST();

  ADD_TEST_CASE("bad thread id", stop_thread(1234, 321) == ERR_NOT_FOUND);

  ADD_TEST_CASE("stopping right", !stop_thread(id, 321));

  uint64_t prev_counter = *counter;
  for (volatile int i = 0;
       prev_counter == *counter && i < WAIT_ITERATIONS; i++) { }
  ADD_TEST_CASE("making sure it stopped", prev_counter == *counter);

  ADD_TEST_CASE("stopping stopped", stop_thread(id, 321) == ERR_BAD_STATE);

  ADD_TEST_CASE("resuming stopped", resume_thread(id) == ERR_BAD_STATE);

  struct thread_data *thrd;
  ADD_TEST_CASE("detaching right",
                !detach_thread(id, &thrd) &&
                thrd->state == THREAD_STATE_STOPPED);

  END_TEST();
}

DEFINE_SUBTEST(scheduler_sanity, struct mem_pool *thread_pool) {
  BEGIN_TEST();

  volatile uint64_t counter = UINT64_MAX;
  struct thread_data *thrd = create_test_thread(thread_pool, counter_proc,
                                                (uint64_t)&counter);
  if (!thrd)
    return false;

  thread_id id;
  ADD_TEST(attach_thread, thrd, &id);
  ADD_TEST(resume_thread, id, &counter);
  ADD_TEST(pause_thread, id, &counter);
  ADD_TEST(detach_thread, &id, &counter);
  ADD_TEST(stop_thread, id, &counter);

  if (passed) {
    LOG_INFO("running on BSP CPU only...");
    set_test_thread_affinity(thrd, true);
  }

  ADD_TEST(attach_thread, thrd, &id);
  ADD_TEST(resume_thread, id, &counter);
  ADD_TEST(pause_thread, id, &counter);
  ADD_TEST(detach_thread, &id, &counter);
  ADD_TEST(stop_thread, id, &counter);

  destroy_test_thread(thread_pool, thrd);
  END_TEST();
}

DEFINE_TEST(scheduler, struct mem_pool *thread_pool) {
  BEGIN_TEST();
  ADD_TEST(scheduler_sanity, thread_pool);
  END_TEST();
}
