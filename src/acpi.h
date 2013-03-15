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
} PACKED;

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
} PACKED;

struct acpi_rsdt {
  struct acpi_header header;
  uint32_t tables[];
} PACKED;

struct acpi_xsdt {
  struct acpi_header header;
  uint64_t tables[];
} PACKED;

struct acpi_madt {
  struct acpi_header header;
  uint32_t lapic_addr;
  uint32_t pcat_compat : 1;
  uint32_t reserved : 31;
} PACKED;

#define ACPI_MADT_LAPIC_TYPE 0
#define ACPI_MADT_IOAPIC_TYPE 1
#define ACPI_MADT_INTSRCOVER_TYPE 2

struct acpi_entry_header {
  uint8_t type;
  uint8_t length;
};

struct acpi_madt_lapic {
  struct acpi_entry_header header;
  uint8_t acpi_cpuid;
  uint8_t apic_id;
  uint32_t enabled : 1;
  uint32_t reserved : 31;
} PACKED;

struct acpi_madt_ioapic {
  struct acpi_entry_header header;
  uint8_t ioapic_id;
  uint8_t reserved;
  uint32_t ioapic_addr;
  uint32_t int_base;
} PACKED;

struct acpi_madt_intsrcover {
  struct acpi_entry_header header;
  uint8_t bus_src;
  uint8_t irq_src;
  uint32_t int_pin;
  uint16_t polarity : 2;
  uint16_t trigger_mode : 2;
  uint16_t reserved : 12;
} PACKED;

struct acpi_srat {
  struct acpi_header header;
  uint32_t reserved0;
  uint64_t reserved1;
} PACKED;

#define ACPI_SRAT_LAPIC_TYPE 0
#define ACPI_SRAT_MEMORY_TYPE 1
#define ACPI_SRAT_LX2APIC_TYPE 2

struct acpi_srat_lapic {
  struct acpi_entry_header header;
  uint8_t prox_domain0;
  uint8_t apic_id;
  uint32_t enabled : 1;
  uint32_t reserved : 31;
  uint8_t lsapic_eid;
  uint16_t prox_domain1;
  uint8_t prox_domain2;
  uint32_t clock_domain;
} PACKED;

struct acpi_srat_memory {
  struct acpi_entry_header header;
  uint32_t prox_domain;
  uint16_t reserved0;
  uint64_t base_addr;
  uint64_t length;
  uint32_t reserved1;
  uint32_t enabled : 1;
  uint32_t hot_plug : 1;
  uint32_t non_volat : 1;
  uint32_t reserved2 : 29;
  uint64_t reserved3;
} PACKED;

struct acpi_srat_lx2apic {
  struct acpi_entry_header header;
  uint16_t reserved0;
  uint32_t prox_domain;
  uint32_t x2apic_id;
  uint32_t enabled : 1;
  uint32_t reserved1 : 31;
  uint32_t clock_domain;
  uint32_t reserved2;
} PACKED;

const struct acpi_rsdp *get_acpi_rsdp(void);
const struct acpi_madt *get_acpi_madt(void);
const struct acpi_srat *get_acpi_srat(void);

static inline const struct acpi_rsdt *get_acpi_rsdt(void) {
  const struct acpi_rsdp *rsdp = get_acpi_rsdp();
  return rsdp ? (struct acpi_rsdt*)(uint64_t)rsdp->rsdt_addr : NULL;
}

static inline const struct acpi_xsdt *get_acpi_xsdt(void) {
  const struct acpi_rsdp *rsdp = get_acpi_rsdp();
  return rsdp ? (struct acpi_xsdt*)rsdp->xsdt_addr : NULL;
}

// ignores type if a given type parameter is equal to -1
bool get_next_acpi_entry(const void *table, void *entry_pptr, int type);

void init_acpi(void);

#endif
