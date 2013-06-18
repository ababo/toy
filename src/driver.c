#include "driver.h"
#include "sync.h"

#define DRIVER_MAGIC 0x0288419716939937

static int stamp;
static struct driver *tail;
static struct mutex mutex;

err_code add_driver(struct driver *driver, driver_id *id) {
  if (driver->magic == DRIVER_MAGIC) // already attached
    return ERR_BAD_STATE;

  err_code err;
  if (!(err = acquire_mutex(&mutex))) {

    driver->magic = DRIVER_MAGIC;
    assign_stamp_id_stamp(&driver->stamp, &stamp);
    driver->prev = NULL, driver->next = tail;
    tail->prev = driver, tail = driver;

    if (id)
      *id = get_stamp_id(driver, driver->stamp);

    release_mutex(&mutex);
  }
  return err;
}

err_code remove_driver(driver_id id) {
  err_code err;
  if (!(err = acquire_mutex(&mutex))) {

    WITH_STAMP_ID(struct driver, drv, id, DRIVER_MAGIC) {
      if (drv->prev)
        drv->prev->next = drv->next;
      if (drv->next)
        drv->next->prev = drv->prev;
      if (tail == drv)
        tail = tail->next;
      drv->magic = 0;
    }

    release_mutex(&mutex);
  }
  return err;
}

err_code get_next_driver(driver_id *id, int type) {
  if (!id)
    return ERR_BAD_INPUT;

  err_code err;
  if (!(err = acquire_mutex(&mutex))) {

    if (*id) {
      WITH_STAMP_ID(struct driver, drv, *id, DRIVER_MAGIC) {
        do { drv = drv->next; }
        while (type != -1 && drv && drv->type != type);
        *id = drv ? get_stamp_id(drv, drv->stamp) : 0;
      }
    }
    else
      *id = tail ? get_stamp_id(tail, tail->stamp) : 0;

    release_mutex(&mutex);
  }
  return !err && !*id ? ERR_NO_MORE : err;
}

void __init_builtin_drivers(void);

void init_drivers(void) {
  if (create_mutex(&mutex))
    PANIC("Failed to create a mutex");
  __init_builtin_drivers();
}
