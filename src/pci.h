#ifndef PCI_H
#define PCI_H

#include "util.h"

#define PCI_HEADER_DEVICE 0
#define PCI_HEADER_PCI_TO_PCI_BRIDGE 1
#define PCI_HEADER_PCI_TO_CARDBUS_BRIDGE 2

#define PCI_REG_BAR0 0x10
#define PCI_REG_BAR1 0x14
#define PCI_REG_BAR2 0x18
#define PCI_REG_BAR3 0x1C
#define PCI_REG_BAR4 0x20
#define PCI_REG_BAR5 0x24

struct pci_addr {
  uint32_t zero : 2;
  uint32_t reg : 6;
  uint32_t func : 3;
  uint32_t dev : 5;
  uint32_t bus : 8;
  uint32_t reserved : 7;
  uint32_t one : 1;
};

struct pci_header_type {
  uint8_t type : 7;
  uint8_t mf : 1;
};

struct pci_header_bist {
  uint8_t completion : 4;
  uint8_t reserved : 2;
  uint8_t start : 1;
  uint8_t capable : 1;
};

struct pci_header {
  uint16_t vendor_id;
  uint16_t device_id;
  uint16_t command;
  uint16_t status;
  uint8_t revision_id;
  uint8_t prog_if;
  uint8_t subclass;
  uint8_t class;
  uint8_t cache_line_size;
  uint8_t latency_timer;
  struct pci_header_type header_type;
  struct pci_header_bist bist;
};

// reg -1 means register is taken from dev
uint32_t read_pci(struct pci_addr dev, int reg);
void write_pci(struct pci_addr dev, int reg, uint32_t value);

bool get_next_pci_dev(struct pci_addr *dev); // start passing dev with .one = 0
void get_pci_header(struct pci_addr dev, struct pci_header *header);

void init_pci(void);

#endif // PCI_H
