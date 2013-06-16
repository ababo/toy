#include "../common.h"
#include "../sync.h"
#include "ahci.h"
#include "page_map.h"
#include "pci.h"

#define DRIVE_MAGIC 0x5105820974944592

#define MAX_DRIVES 8
#define MAX_PORTS 32

#define DET_PRESENT 3
#define IPM_ACTIVE 1

#define SIG_ATA 0x00000101
#define SIG_ATAPI 0xEB140101
#define	SIG_SEMB 0xC33C0101
#define	SIG_PM 0x96690101

struct drive {
  uint64_t magic;
  struct drive *next;
  struct mutex lock;
  int stamp;
  int hba_pci_address : 24;
  int hba_port : 5;
  int connected : 1;
  int in_use : 1;
  int used : 1;

};

struct port {
  uint32_t clb, clbu, fb, fbu, is, ie, cmd, reserved0, tfd, sig, ssts, sctl;
  uint32_t serr, sact, ci, sntf, fbs;
  uint32_t reserved1[11], vendor[4];
};

struct hba {
  uint32_t cap, ghc, is, pi, vs, ccc_ctl, ccc_pts, em_loc, em_ctl, cap2, bohc;
  uint8_t reserved[0xA0-0x2C], vendor[0x100-0xA0];
  struct port ports[];
};

static struct drive drives[MAX_DRIVES];
static struct ahci_driver driver;
static struct mutex lock;

const struct ahci_driver *get_ahci_driver(void) {
  return &driver;
}

static void next_drive(int hba_pci_addess, struct hba *hba, int port) {
  LOG_DEBUG("AHCI HBA %X:%X.%X: SATA drive detected on port %d",
            get_pci_bus(hba_pci_addess), get_pci_slot(hba_pci_addess),
            get_pci_func(hba_pci_addess), port);

}

static void next_hba(int device) {
  uint64_t bar5 = read_pci_field(device, PCI_FIELD_BAR5) & ~0x1FFF;
  map_page(bar5, bar5, PAGE_MAPPING_WRITE | PAGE_MAPPING_PWT |
           PAGE_MAPPING_PCD, 0);
  struct hba *hba = (struct hba*)bar5;

  for (uint32_t pi = hba->pi, port = 0; port < MAX_PORTS; port++)
    if (BIT_ARRAY_GET(&pi, port)) {
      uint32_t det = INT_BITS(hba->ports[port].ssts, 0, 3);
      uint32_t ipm = INT_BITS(hba->ports[port].ssts, 8, 11);
      uint32_t sig = hba->ports[port].sig;
      if (det == DET_PRESENT && ipm == IPM_ACTIVE &&
          sig != SIG_ATAPI && sig != SIG_SEMB && sig != SIG_PM)
        next_drive(device, hba, port);
    }
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
