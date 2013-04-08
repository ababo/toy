#include "../schedule.h"
#include "../memory.h"
#include "test.h"

#define STACK_SIZE 0x1000
#define DEFAULT_PRIORITY 3;

static struct thread_data *create_thread(thread_proc proc, uint64_t input) {
  struct thread_data *thrd = kmalloc(sizeof(*thrd) + STACK_SIZE);
  if (!thrd) {
    LOG_ERROR("Failed to allocate memory");
    return NULL;
  }

  memset(thrd, 0, sizeof(*thrd));
  thrd->stack = (uint8_t*)(thrd + 1);
  thrd->stack_size = STACK_SIZE;
  for (int i = 0; i < THREAD_AFFINITY_SIZE; i++)
    thrd->affinity[0] = UINT64_MAX;
  thrd->priority = DEFAULT_PRIORITY;
  thrd->fixed_priority = true;
  set_thread_context(thrd, proc, input);

  return thrd;
}

static void destroy_thread(struct thread_data *thread) {
  kfree(thread);
}

static uint64_t factorial_proc(uint64_t input) {
  uint64_t output = 1;
  for (uint64_t i = 2; i <= input; output *= i++);
  LOG_INFO("%d!=%ld", input, output);
  return output;
}

DEFINE_SUBTEST(schedule_attach_thread, struct thread_data *thread,
               thread_id *id) {
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

  ADD_TEST_CASE("attaching normally", !attach_thread(thread, id));

  ADD_TEST_CASE("attaching already attached",
                attach_thread(thread, id) == ERR_BAD_STATE);

  END_TEST();
}
/*
DEFINE_SUBTEST(schedule_resume_thread, thread_id id) {
  BEGIN_TEST();

  LOG_DEBUG("err: %d", resume_thread(id));

  END_TEST();
}
*/
DEFINE_TEST(schedule) {
  BEGIN_TEST();

  struct thread_data *thrd = create_thread(factorial_proc, 15);
  if (!thrd)
    return false;

  thread_id id;
  ADD_TEST(schedule_attach_thread, thrd, &id);
  //ADD_TEST(schedule_resume_thread, id);

  destroy_thread(thrd);

  END_TEST();
}
