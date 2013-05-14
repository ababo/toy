#ifndef PCI_H
#define PCI_H

#include "util.h"

struct pci_field {
  uint8_t reg, low_bit, high_bit;
};

#define PCI_FIELD_VENDOR_ID (struct pci_field) { 0, 0, 15 }
#define PCI_FIELD_DEVICE_ID (struct pci_field) { 0, 16, 31 }
#define PCI_FIELD_COMMAND (struct pci_field) { 1, 0, 15 }
#define PCI_FIELD_STATUS (struct pci_field) { 1, 16, 31 }
#define PCI_FIELD_REVISION_ID (struct pci_field) { 2, 0, 7 }
#define PCI_FIELD_PROG_IF (struct pci_field) { 2, 8, 15 }
#define PCI_FIELD_SUBCLASS (struct pci_field) { 2, 16, 23 }
#define PCI_FIELD_CLASS (struct pci_field) { 2, 24, 31 }
#define PCI_FIELD_CACHE_LINE_SIZE (struct pci_field) { 3, 0, 7 }
#define PCI_FIELD_LATENCY_TIMER (struct pci_field) { 3, 8, 15 }
#define PCI_FIELD_HEADER_TYPE (struct pci_field) { 3, 16, 22 }
#define PCI_FIELD_MULTIPLE_FUNCTIONS (struct pci_field) { 3, 23, 23 }
#define PCI_FIELD_BIST_COMPLETION_CODE (struct pci_field) { 3, 24, 27 }
#define PCI_FIELD_BIST_START (struct pci_field) { 3, 30, 30 }
#define PCI_FIELD_BIST_CAPABLE (struct pci_field) { 3, 31, 31 }
#define PCI_FIELD_SECONDARY_BUS (struct pci_field) { 6, 8, 15 }

#define PCI_HEADER_DEVICE 0
#define PCI_HEADER_PCI_TO_PCI_BRIDGE 1
#define PCI_HEADER_PCI_TO_CARDBUS_BRIDGE 2

struct pci_device {
  uint8_t bus, slot, func;
};

uint32_t read_pci_field(struct pci_device device, struct pci_field field);
void write_pci_field(struct pci_device device, struct pci_field field,
                     uint32_t value);

struct pci_type {
  uint8_t class, subclass;
};

static inline bool has_pci_type(struct pci_device device,
                                struct pci_type type) {
  return read_pci_field(device, PCI_FIELD_CLASS) == type.class &&
    read_pci_field(device, PCI_FIELD_SUBCLASS) == type.subclass;
}

#define PCI_TYPE_PCI_TO_PCI_BRIDGE (struct pci_type) { 0x06, 0x04 }
#define PCI_TYPE_SERIAL_ATA (struct pci_type) { 0x01, 0x06 }

typedef void (*pci_scan_proc)(struct pci_device device);
void scan_pci(pci_scan_proc proc);

void init_pci(void);

#endif // PCI_H
