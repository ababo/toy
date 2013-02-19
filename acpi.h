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
};

struct acpi_rsdt {
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oem_id[6];
  uint64_t oem_model_id;
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
  uint32_t tables[1];
};

struct acpi_xsdt {
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oem_id[6];
  uint64_t oem_model_id;
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
  uint64_t tables[1];
} __attribute__((packed));

const struct acpi_rsdp *get_acpi_rsdp(void);

static inline const struct acpi_rsdt *get_acpi_rsdt(void) {
  const struct acpi_rsdp *rsdp = get_acpi_rsdp();
  return rsdp ? (struct acpi_rsdt*)(uint64_t)rsdp->rsdt_addr : NULL;
}

static inline const struct acpi_xsdt *get_acpi_xsdt(void) {
  const struct acpi_rsdp *rsdp = get_acpi_rsdp();
  return rsdp ? (struct acpi_xsdt*)rsdp->xsdt_addr : NULL;
}

void init_acpi(void);

#endif
