#include "../common.h"
#include "ahci.h"

static bool initialized;
static struct ahci_driver driver;

const struct ahci_driver *get_ahci_driver(void) {
  return initialized ? &driver : NULL;
}

err_code init_ahci(void) {
  err_code err = ERR_NONE;


  if (err) {
    LOG_ERROR("failed with error %d", err);
  }
  else {
    add_driver((struct driver*)&driver);
    LOG_DEBUG("done");
  }
  initialized = !err;
  return err;
}
