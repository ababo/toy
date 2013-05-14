#include "pci.h"

static inline uint32_t get_addr(struct pci_device device,
                                struct pci_field field) {
  return 0x80000000 | (device.bus << 16) | (device.slot << 11) |
    (device.func << 8) | (field.reg << 2);
}

#define ADDRESS_PORT 0xCF8
#define VALUE_PORT 0xCFC

uint32_t read_pci_field(struct pci_device device, struct pci_field field) {
  outl(ADDRESS_PORT, get_addr(device, field));
  return INT_BITS(inl(VALUE_PORT), field.low_bit, field.high_bit);
}

void write_pci_field(struct pci_device device, struct pci_field field,
                     uint32_t value) {
  outl(ADDRESS_PORT, get_addr(device, field));
  if (field.low_bit > 0 || field.high_bit < 31) {
    uint32_t prev = inl(VALUE_PORT), next = 0;
    if (field.low_bit > 0)
      next |= INT_BITS(prev, 0, field.low_bit - 1);
    next |= (value << field.low_bit);
    if (field.high_bit < 31)
      next |= (INT_BITS(prev, field.high_bit + 1, 31) << (field.high_bit + 1));
    value = next;
  }
  outl(VALUE_PORT, value);
}

static void scan_bus(pci_scan_proc proc, int bus);

static void scan_func(pci_scan_proc proc, int bus, int slot, int func) {
  struct pci_device dev = { bus, slot, func };
  proc(dev);

  if (has_pci_type(dev, PCI_TYPE_PCI_TO_PCI_BRIDGE))
    scan_bus(proc, read_pci_field(dev, PCI_FIELD_SECONDARY_BUS));
}

static void scan_slot(pci_scan_proc proc, int bus, int slot) {
  struct pci_device dev = { bus, slot, 0 };
  if (read_pci_field(dev, PCI_FIELD_VENDOR_ID) == 0xFFFF)
    return;

  scan_func(proc, bus, slot, 0);
  if (!read_pci_field(dev, PCI_FIELD_MULTIPLE_FUNCTIONS))
    return;

  for (dev.func = 1; dev.func < 8; dev.func++)
    if (read_pci_field(dev, PCI_FIELD_VENDOR_ID) != 0xFFFF)
      scan_func(proc, bus, slot, dev.func);
}

static void scan_bus(pci_scan_proc proc, int bus) {
  for (int slot = 0; slot < 32; slot++)
    scan_slot(proc, bus, slot);
}

void scan_pci(pci_scan_proc proc) {
  scan_bus(proc, 0);

  struct pci_device dev = { 0, 0, 0 };
  if (!read_pci_field(dev, PCI_FIELD_MULTIPLE_FUNCTIONS))
    return;

  for (dev.func = 1; dev.func < 8; dev.func++)
    if (read_pci_field(dev, PCI_FIELD_VENDOR_ID) != 0xFFFF)
      scan_bus(proc, dev.func);
    else
      break;
}

static void scan_proc(struct pci_device device) {
  LOG_INFO("PCI device found (bus: %X, slot: %X, func: %X)",
           device.bus, device.slot, device.func);
}

void init_pci(void) {
  scan_pci(scan_proc);
}
