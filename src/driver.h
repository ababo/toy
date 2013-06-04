#ifndef __DRIVER_H
#define __DRIVER_H

#include "common.h"
#include "retained.h"

#define DRIVER_TYPE_STORAGE 1

typedef uint64_t device_id;

struct idriver {
  struct retained retained;
  int type;
  bool (*get_next_device)(device_id *device); // start with device == 0
  err_code (*start_device)(device_id device);
  err_code (*stop_device)(device_id device);
  
};

struct istorage_driver {
  struct idriver idriver;
  int block_size;
  
};

bool get_next_driver(struct idriver **driver); // start with *driver == NULL

void add_driver(void *driver);
void remove_driver(void *driver);

void init_drivers(void);

#endif // __DRIVER_H
