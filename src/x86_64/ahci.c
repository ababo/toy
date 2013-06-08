#include "../common.h"
#include "../sync.h"
#include "ahci.h"
#include "pci.h"

#define DRIVE_MAGIC 0x5105820974944592

struct drive {
  uint64_t magic;
  struct drive *next;
  struct mutex lock;
  int hba_pci_address : 24;
  int hba_slot : 8;
  uint8_t scanned : 1;
  
};

static struct mem_pool drive_pool;
static struct drive *drives_tail;
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

  for (struct drive *drive = drives_tail; drive; drive = drive->next)
    drive->scanned = false;

  scan_pci(next_hba, PCI_TYPE_SERIAL_ATA);



  release_mutex(&lock);
  return err;
}

void init_ahci(void) {
  driver.storage_driver.driver = (struct driver) {
    .type = DRIVER_TYPE_STORAGE, .scan_devices = scan_devices
  };

  if (create_mutex(&lock))
    LOG_ERROR("failed to create mutex")
  else if (create_mem_pool(sizeof(struct drive), &drive_pool))
    LOG_ERROR("failed to create drive pool")
  else if (add_driver((struct driver*)&driver))
    LOG_ERROR("failed to add driver")
  else if (scan_devices())
    LOG_ERROR("failed to scan drives")
  else
    LOG_DEBUG("done");
}

void dispose_ahci(void) {

}
