#include "cpu.h"
#include "pci.h"

static inline uint32_t get_addr(int device, int field) {
  int reg = (uint8_t)(field >> 16);
  return 0x80000000 | (get_pci_bus(device) << 16) |
    (get_pci_slot(device) << 11) | (get_pci_func(device) << 8) | (reg << 2);
}

#define ADDRESS_PORT 0xCF8
#define VALUE_PORT 0xCFC

uint32_t read_pci_field(int device, int field) {
  int high_bit = (uint8_t)field, low_bit = (uint8_t)(field >> 8);
  outl(ADDRESS_PORT, get_addr(device, field));
  return INT_BITS(inl(VALUE_PORT), low_bit, high_bit);
}

void write_pci_field(int device, int field, uint32_t value) {
  int high_bit = (uint8_t)field, low_bit = (uint8_t)(field >> 8);
  outl(ADDRESS_PORT, get_addr(device, field));
  if (low_bit > 0 || high_bit < 31) {
    uint32_t prev = inl(VALUE_PORT), next = 0;
    if (low_bit > 0)
      next |= INT_BITS(prev, 0, low_bit - 1);
    next |= (value << low_bit);
    if (high_bit < 31)
      next |= (INT_BITS(prev, high_bit + 1, 31) << (high_bit + 1));
    value = next;
  }
  outl(VALUE_PORT, value);
}

static void scan_bus(pci_scan_proc proc, int bus);

static void scan_func(pci_scan_proc proc, int bus, int slot, int func) {
  int dev = get_pci_device(bus, slot, func);
  proc(dev);

  if (get_pci_type(dev) == PCI_TYPE_PCI_TO_PCI_BRIDGE)
    scan_bus(proc, read_pci_field(dev, PCI_FIELD_SECONDARY_BUS));
}

static void scan_slot(pci_scan_proc proc, int bus, int slot) {
  int dev = get_pci_device(bus, slot, 0);
  if (read_pci_field(dev, PCI_FIELD_VENDOR_ID) == 0xFFFF)
    return;

  scan_func(proc, bus, slot, 0);
  if (!read_pci_field(dev, PCI_FIELD_MULTIPLE_FUNCTIONS))
    return;

  for (int func = 1; func < 8; func++) {
    int dev = get_pci_device(bus, slot, func);
    if (read_pci_field(dev, PCI_FIELD_VENDOR_ID) != 0xFFFF)
      scan_func(proc, bus, slot, func);
  }
}

static void scan_bus(pci_scan_proc proc, int bus) {
  for (int slot = 0; slot < 32; slot++)
    scan_slot(proc, bus, slot);
}

void scan_pci(pci_scan_proc proc) {
  scan_bus(proc, 0);

  if (!read_pci_field(0, PCI_FIELD_MULTIPLE_FUNCTIONS))
    return;

  for (int func = 1; func < 8; func++) {
    int dev = get_pci_device(0, 0, func);
    if (read_pci_field(dev, PCI_FIELD_VENDOR_ID) != 0xFFFF)
      scan_bus(proc, func);
    else
      break;
  }
}

static void scan_proc(int device) {
  LOG_DEBUG("PCI device found (address: %X:%X.%X, type: %X)",
            get_pci_bus(device), get_pci_slot(device), get_pci_func(device),
            get_pci_type(device));
}

void init_pci(void) {
  scan_pci(scan_proc);
}
