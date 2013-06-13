#include "driver.h"
#include "sync.h"

#define DRIVER_MAGIC 0x0288419716939937

static int stamp;
static struct driver *tail;
static struct mutex mutex;

err_code add_driver(struct driver *driver, driver_id *id) {
  if (driver->magic == DRIVER_MAGIC) // already attached
    return ERR_BAD_STATE;

  err_code err = ERR_NONE;
  acquire_mutex(&mutex);

  driver->magic = DRIVER_MAGIC;
  driver->stamp = stamp;
  stamp = (stamp + 1) % (1 << PACKED_POINTER_DATA_BITS);
  driver->prev = NULL, driver->next = tail;
  tail->prev = driver, tail = driver;

  if (id)
    *id = pack_pointer(driver, driver->stamp);

  release_mutex(&mutex);
  return err;
}

#define CAST_TO_DRIVER(drv, id)                         \
  int stamp;                                            \
  struct driver *drv = unpack_pointer(id, &stamp);      \
  if (drv->magic != DRIVER_MAGIC)                       \
    err = ERR_NOT_FOUND;                                \
  else if (drv->stamp != stamp)                         \
    err = ERR_EXPIRED;

err_code remove_driver(driver_id id) {
  err_code err = ERR_NONE;
  acquire_mutex(&mutex);

  CAST_TO_DRIVER(drv, id);
  if (!err) {
    if (drv->prev)
      drv->prev->next = drv->next;
    if (drv->next)
      drv->next->prev = drv->prev;
    if (tail == drv)
      tail = tail->next;
    drv->magic = 0;
  }

  release_mutex(&mutex);
  return err;
}

err_code get_next_driver(driver_id *id, int type) {
  if (!id)
    return ERR_BAD_INPUT;

  err_code err = ERR_NONE;
  acquire_mutex(&mutex);

  if (*id) {
    CAST_TO_DRIVER(drv, *id);
    if (!err) {
      do { drv = drv->next; }
      while (type != -1 && drv && drv->type != type);
      *id = drv ? pack_pointer(drv, drv->stamp) : 0;
    }
  }
  else
    *id = tail ? pack_pointer(tail, tail->stamp) : 0;

  release_mutex(&mutex);
  return err;
}

void __init_builtin_drivers(void);

void init_drivers(void) {
  if (create_mutex(&mutex))
    PANIC("Failed to create a mutex");
  __init_builtin_drivers();
}
