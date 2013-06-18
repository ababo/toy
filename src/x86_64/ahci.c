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

struct drive {
  uint64_t magic;
  struct drive *next;
  struct mutex lock;
  struct hba *hba;
  int stamp;
  int hba_pci_address : 24;
  int hba_port : 5;
  int scanned : 1;
  int in_use : 1;
  int used : 1;

};

static err_code scan_error;
static bool scan_add_drives;
static struct drive drives[MAX_DRIVES];
static struct ahci_driver driver;
static struct mutex lock;
static int stamp;

const struct ahci_driver *get_ahci_driver(void) {
  return &driver;
}

static void add_drive(int hba_pci_address, struct hba *hba, int hba_port) {
  for (int i = 0; i < MAX_DRIVES; i++)
    if (!drives[i].in_use) {
      drives[i].hba_pci_address = hba_pci_address;
      drives[i].hba = hba, drives[i].hba_port = hba_port;
      drives[i].used = drives[i].in_use = true;
      drives[i].magic = DRIVE_MAGIC, drives[i].stamp = stamp;
      stamp = (stamp + 1) % (1 << PACKED_POINTER_DATA_BITS);


      LOG_INFO("AHCI HBA %X:%X.%X: SATA drive added on port %d",
               get_pci_bus(hba_pci_address), get_pci_slot(hba_pci_address),
               get_pci_func(hba_pci_address), hba_port);
      return;
    }

  scan_error = ERR_NO_MORE;
}

static void scan_next_drive(int hba_pci_address, struct hba *hba, int port) {
  if (scan_error)
    return;

  for (int i = 0; i < MAX_DRIVES; i++)
    if (drives[i].in_use && drives[i].hba_pci_address == hba_pci_address &&
        drives[i].hba_port == port) {
      drives[i].scanned = true;
      return;
    }

  if (scan_add_drives)
    add_drive(hba_pci_address, hba, port);
}

static void scan_next_hba(int device) {
  if (scan_error)
    return;

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
        scan_next_drive(device, hba, port);
    }
}

static void remove_drive(struct drive *drive) {
  if (!(scan_error = acquire_mutex(&drive->lock))) {
    drives->magic = 0;
    drive->in_use = false;

    release_mutex(&drive->lock);
  }

  LOG_INFO("AHCI HBA %X:%X.%X: SATA drive removed from port %d",
           get_pci_bus(drive->hba_pci_address),
           get_pci_slot(drive->hba_pci_address),
           get_pci_func(drive->hba_pci_address), drive->hba_port);
}

static err_code scan_devices(void) {
  if (!(scan_error = acquire_mutex(&lock))) {

    for (int i = 0; i < MAX_DRIVES; i++)
      if (drives[i].in_use)
        drives[i].scanned = false;

    scan_add_drives = false;
    scan_pci(scan_next_hba, PCI_TYPE_SERIAL_ATA);

    if (!scan_error) {
      for (int i = 0; i < MAX_DRIVES; i++)
        if (drives[i].in_use && !drives[i].scanned)
          remove_drive(&drives[i]);

      scan_add_drives = true;
      scan_pci(scan_next_hba, PCI_TYPE_SERIAL_ATA);
    }

    release_mutex(&lock);
  }
  return scan_error;
}

static err_code get_next_device(device_id *id) {
  if (id)
    return ERR_BAD_INPUT;

  err_code err;
  if (!(err = acquire_mutex(&lock))) {

    int i = 0;
    if (*id) {
      WITH_STAMP_ID(struct drive, drv, *id, DRIVE_MAGIC)
        i = drv - drives + 1;
    }

    for (; i < MAX_DRIVES; i++)
      if (drives[i].in_use) {
        *id = get_stamp_id(&drives[i], drives[i].stamp);
        break;
      }

    if (i >= MAX_DRIVES)
      err = ERR_NO_MORE;

    release_mutex(&lock);
  }
  return err;
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
