#ifndef PCI_H
#define PCI_H

#include "util.h"

#define PCI_HEADER_DEVICE 0
#define PCI_HEADER_PCI_TO_PCI_BRIDGE 1
#define PCI_HEADER_PCI_TO_CARDBUS_BRIDGE 2

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
#define PCI_FIELD_CLASS (struct pci_field) { 2, 23, 31 }
#define PCI_FIELD_CACHE_LINE_SIZE (struct pci_field) { 3, 0, 7 }
#define PCI_FIELD_LATENCY_TIMER (struct pci_field) { 3, 8, 15 }
#define PCI_FIELD_HEADER_TYPE (struct pci_field) { 3, 16, 23 }
#define PCI_FIELD_BIST (struct pci_field) { 3, 23, 31 }

struct pci_device {
  uint8_t bus, slot, func;
};

uint32_t read_pci_field(struct pci_device device, struct pci_field field);
void write_pci_field(struct pci_device device, struct pci_field field,
                     uint32_t value);

typedef void (*pci_scan_proc)(struct pci_device device);
void scan_pci(pci_scan_proc proc);

void init_pci(void);

#endif // PCI_H
