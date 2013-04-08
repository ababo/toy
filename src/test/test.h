#ifndef TEST_H
#define TEST_H

#include "../util.h"

#define DEFINE_TEST(name, ...)                          \
  bool test_##name(bool tolerate_errors, ##__VA_ARGS__)

#define DEFINE_SUBTEST(name, ...)               \
  static DEFINE_TEST(name, ##__VA_ARGS__)

#define BEGIN_TEST() bool passed = true;

#define END_TEST()                              \
  LOG_INFO(passed ? "passed" : "FAILED");       \
  return passed;

#define ADD_TEST(name, ...)                                             \
  if (tolerate_errors || passed)                                        \
    passed = passed && test_##name(tolerate_errors, ##__VA_ARGS__);

#define ADD_TEST_CASE(case, result)                          \
  if (tolerate_errors || passed) {                           \
    bool temp = result; passed = passed && temp;             \
    LOG_INFO("%s: %s", case, temp ? "passed" : "FAILED");    \
  }

DEFINE_TEST(all);
DEFINE_TEST(schedule);

#endif // TEST_H
