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

void scan_pci(UNUSED pci_scan_proc proc) {

}

static void scan_proc(struct pci_device device) {
  LOG_INFO("PCI device found (bus: %d, slot: %d, func: %d)",
           device.bus, device.slot, device.func);
}

void init_pci(void) {
  scan_pci(scan_proc);
}
