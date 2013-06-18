#ifndef __DRIVER_H
#define __DRIVER_H

#include "common.h"
#include "stamp_id.h"

#define DRIVER_TYPE_STORAGE 1

typedef stamp_id driver_id;
typedef stamp_id device_id;

struct driver {
  INTERNAL uint64_t magic;
  INTERNAL struct driver *prev, *next;
  INTERNAL int stamp;
  int type;
  err_code (*scan_devices)(void);
  // start enumerating with *id == 0
  // returns ERR_NO_MORE when no more devices remain
  // returns ERR_EXPIRED in case of need to restart enumeration
  err_code (*get_next_device)(device_id *id);
  err_code (*start_device)(device_id device);
  err_code (*stop_device)(device_id device);

};

struct storage_driver {
  struct driver driver;

};

static inline const struct driver *get_driver(driver_id id) {
  return get_stamp_id_ptr(id);
}

err_code add_driver(struct driver *driver, driver_id *id);
err_code remove_driver(driver_id id);

// start enumerating with *id == 0
// ignores driver type if it is equal to -1
// returns ERR_NO_MORE when no more drivers remain
// returns ERR_EXPIRED in case of need to restart enumeration
err_code get_next_driver(driver_id *id, int type);

void init_drivers(void);

#endif // __DRIVER_H
