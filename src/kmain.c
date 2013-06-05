#include "common.h"
#include "driver.h"
#include "test/test.h"

uint64_t kmain(UNUSED uint64_t input) {
  init_drivers();
  return test_all(false);
}
