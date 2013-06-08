#include "common.h"
#include "driver.h"
#include "test/test.h"

uint64_t kmain(UNUSED uint64_t input) {
  init_drivers();

  const struct driver *driver = NULL;
  if (!get_next_driver(&driver, DRIVER_TYPE_STORAGE))
    PANIC("no storage driver found");

  return 0; //test_all(false);
}
