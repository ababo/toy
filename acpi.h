#ifndef ACPI_H
#define ACPI_H

#include "util.h"

struct acpi_rsdp {
  char signature[8];
  uint8_t checksum;
  char oem_id[6];
  uint8_t revision;
  uint32_t rsdt_addr;
  uint32_t length;
  uint64_t xsdt_addr;
  uint8_t checksum_ex;
  uint8_t reserved[3];
} __attribute__((packed));

struct acpi_header {
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oem_id[6];
  uint64_t oem_table_id;
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
} __attribute__((packed));

struct acpi_rsdt {
  struct acpi_header header;
  uint32_t tables[];
} __attribute__((packed));

struct acpi_xsdt {
  struct acpi_header header;
  uint64_t tables[];
} __attribute__((packed));

struct acpi_madt {
  struct acpi_header header;
  uint32_t lapic_addr;
  uint32_t flags;
} __attribute__((packed));

#define ACPI_MADT_LAPIC_TYPE 0
#define ACPI_MADT_IOAPIC_TYPE 1
#define ACPI_MADT_INTSRCOVER_TYPE 2

struct acpi_madt_entry_header {
  uint8_t type;
  uint8_t length;
};

struct acpi_madt_lapic_entry {
  struct acpi_madt_entry_header header;
  uint8_t acpi_cpuid;
  uint8_t apic_id;
  uint32_t flags;
} __attribute__((packed));

struct acpi_madt_ioapic_entry {
  struct acpi_madt_entry_header header;
  uint8_t ioapic_id;
  uint8_t reserved;
  uint32_t ioapic_addr;
  uint32_t int_base;
} __attribute__((packed));

struct acpi_madt_intsrcover_entry {
  struct acpi_madt_entry_header header;
  uint8_t bus_src;
  uint8_t irq_src;
  uint32_t int_pin;
  uint16_t flags;
} __attribute__((packed));

const struct acpi_rsdp *get_acpi_rsdp(void);
const struct acpi_madt *get_acpi_madt(void);

static inline const struct acpi_rsdt *get_acpi_rsdt(void) {
  const struct acpi_rsdp *rsdp = get_acpi_rsdp();
  return rsdp ? (struct acpi_rsdt*)(uint64_t)rsdp->rsdt_addr : NULL;
}

static inline const struct acpi_xsdt *get_acpi_xsdt(void) {
  const struct acpi_rsdp *rsdp = get_acpi_rsdp();
  return rsdp ? (struct acpi_xsdt*)rsdp->xsdt_addr : NULL;
}

const struct acpi_madt_entry_header*
next_acpi_madt_entry(const struct acpi_madt_entry_header *entry);

void init_acpi(void);

#endif
