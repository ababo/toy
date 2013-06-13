#include "../common.h"
#include "../sync.h"
#include "ahci.h"
#include "pci.h"

#define DRIVE_MAGIC 0x5105820974944592
#define MAX_DRIVES 8

struct drive {
  uint64_t magic;
  struct drive *next;
  struct mutex lock;
  int stamp;
  int hba_pci_address : 24;
  int hba_slot : 5;
  int connected : 1;
  int in_use : 1;
  int used : 1;

};

static struct drive drives[MAX_DRIVES];
static struct ahci_driver driver;
static struct mutex lock;

const struct ahci_driver *get_ahci_driver(void) {
  return &driver;
}

static void next_hba(int device) {

}

static err_code scan_devices(void) {
  err_code err = ERR_NONE;
  acquire_mutex(&lock);

  for (int i = 0; i < MAX_DRIVES; i++)
    if (drives[i].in_use)
      drives[i].connected = false;

  scan_pci(next_hba, PCI_TYPE_SERIAL_ATA);

  for (int i = 0; i < MAX_DRIVES; i++)
    if (drives[i].in_use && !drives[i].connected) {
      drives[i].in_use = false;

    }

  release_mutex(&lock);
  return err;
}

static err_code get_next_device(device_id *id) {

  return ERR_NO_MORE;
}


void init_ahci(void) {
  memset(drives, 0, sizeof(drives));

  driver.storage_driver.driver = (struct driver) {
    .type = DRIVER_TYPE_STORAGE, .scan_devices = scan_devices,
    .get_next_device = get_next_device,
  };

  if (create_mutex(&lock))
    LOG_ERROR("failed to create mutex")
  else if (scan_devices())
    LOG_ERROR("failed to scan drives")
  else if (add_driver((struct driver*)&driver, NULL))
    LOG_ERROR("failed to add driver")
  else
    LOG_DEBUG("done");
}

void dispose_ahci(void) {
  destroy_mutex(&lock);

  for (int i = 0; i < MAX_DRIVES; i++) {
    if (drives[i].used) {
      destroy_mutex(&drives[i].lock);

    }

  }
}
