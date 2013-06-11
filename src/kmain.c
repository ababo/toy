#include "common.h"
#include "driver.h"
#include "test/test.h"

uint64_t kmain(UNUSED uint64_t input) {
  init_drivers();

  driver_id drv_id = 0;
  if (get_next_driver(&drv_id, DRIVER_TYPE_STORAGE))
    PANIC("no storage driver found");

  device_id dev_id = 0;
  const struct driver *drv = get_driver(drv_id);
  if (drv->get_next_device(&dev_id))
    PANIC("no storage device found");

  return 0; //test_all(false);
}
