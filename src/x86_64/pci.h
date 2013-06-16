#ifndef __X86_64_PCI_H
#define __X86_64_PCI_H

#include "../common.h"

// bytes from high to low: reg, low bit, high bit
#define PCI_FIELD_VENDOR_ID 0x00000F
#define PCI_FIELD_DEVICE_ID 0x00101F
#define PCI_FIELD_COMMAND 0x01000F
#define PCI_FIELD_STATUS 0x01101F
#define PCI_FIELD_REVISION_ID 0x020007
#define PCI_FIELD_PROG_IF 0x02080F
#define PCI_FIELD_SUBCLASS 0x021017
#define PCI_FIELD_CLASS 0x02181F
#define PCI_FIELD_CACHE_LINE_SIZE 0x030007
#define PCI_FIELD_LATENCY_TIMER 0x03080F
#define PCI_FIELD_HEADER_TYPE 0x031016
#define PCI_FIELD_MULTIPLE_FUNCTIONS 0x031717
#define PCI_FIELD_BIST_COMPLETION_CODE 0x03181B
#define PCI_FIELD_BIST_START 0x031E1E
#define PCI_FIELD_BIST_CAPABLE 0x031F1F
#define PCI_FIELD_SECONDARY_BUS 0x06080F
#define PCI_FIELD_BAR5 0x09001F

#define PCI_HEADER_DEVICE 0
#define PCI_HEADER_PCI_TO_PCI_BRIDGE 1
#define PCI_HEADER_PCI_TO_CARDBUS_BRIDGE 2

static inline int get_pci_bus(int device) {
  return (uint8_t)(device >> 16);
}

static inline int get_pci_slot(int device) {
  return (uint8_t)(device >> 8);
}

static inline int get_pci_func(int device) {
  return (uint8_t)device;
}

static inline int get_pci_device(int bus, int slot, int func) {
  return (bus << 16) + (slot << 8) + func;
}

uint32_t read_pci_field(int device, int field);
void write_pci_field(int device, int field, uint32_t value);

static inline int get_pci_type(int device) {
  return (read_pci_field(device, PCI_FIELD_CLASS) << 8) |
    read_pci_field(device, PCI_FIELD_SUBCLASS);
}

// bytes from high to low: class, subclass
#define PCI_TYPE_PCI_TO_PCI_BRIDGE 0x0604
#define PCI_TYPE_SERIAL_ATA 0x0106

typedef void (*pci_scan_proc)(int device);

// ignores type if a given type parameter is equal to -1
void scan_pci(pci_scan_proc proc, int type);

void init_pci(void);

#endif // __X86_64_PCI_H
