#include "driver.h"
#include "sync.h"

#define DRIVER_MAGIC 0x0288419716939937

static struct driver *tail;
static struct mutex mutex;

err_code add_driver(struct driver *driver) {
  if (driver->magic == DRIVER_MAGIC) // already attached
    return ERR_BAD_STATE;

  err_code err = ERR_NONE;
  acquire_mutex(&mutex);

  driver->magic = DRIVER_MAGIC;
  driver->prev = NULL, driver->next = tail;
  tail->prev = driver, tail = driver;

  release_mutex(&mutex);
  return err;
}

err_code remove_driver(struct driver *driver) {
  if (driver->magic != DRIVER_MAGIC)
    return ERR_NOT_FOUND;

  err_code err = ERR_NONE;
  acquire_mutex(&mutex);

  if (driver->prev)
    driver->prev->next = driver->next;
  if (driver->next)
    driver->next->prev = driver->prev;
  if (tail == driver)
    tail = tail->next;
  driver->magic = 0;

  release_mutex(&mutex);
  return err;
}

bool get_next_driver(const struct driver **driver, int type) {
  if (!driver)
    return false;

  if (*driver) {
    if ((*driver)->magic != DRIVER_MAGIC)
      return false;

    do { *driver = (*driver)->next; }
    while (type != -1 && *driver && (*driver)->type != type);
  }
  else
    *driver = tail;

  return !!*driver;
}

void __init_builtin_drivers(void);

void init_drivers(void) {
  if (create_mutex(&mutex))
    PANIC("Failed to create a mutex");
  __init_builtin_drivers();
}
