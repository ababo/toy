#ifndef __DRIVER_H
#define __DRIVER_H

#include "common.h"

#define DRIVER_TYPE_STORAGE 1

typedef uint64_t device_id;

struct driver {
  INTERNAL uint64_t magic;
  INTERNAL struct driver *prev, *next;
  int type;
  err_code (*scan_devices)(void);
  bool (*get_next_device)(device_id *device); // start with device == 0
  err_code (*start_device)(device_id device);
  err_code (*stop_device)(device_id device);
  
};

struct storage_driver {
  struct driver driver;
  
};

err_code add_driver(struct driver *driver);
err_code remove_driver(struct driver *driver);

// start enumerating with *driver == NULL
// ignores type if a given type parameter is equal to -1
bool get_next_driver(const struct driver **driver, int type);

void init_drivers(void);

#endif // __DRIVER_H
