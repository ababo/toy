#include "acpi.h"
#include "mem_map.h"
#include "page_map.h"

#define RSDP_SIGNATURE "RSD PTR "
#define RSDT_SIGNATURE "RSDT"
#define XSDT_SIGNATURE "XSDT"
#define MADT_SIGNATURE "APIC"

static const struct acpi_rsdp *rsdp;
static const struct acpi_madt *madt;

const struct acpi_rsdp *get_acpi_rsdp(void) {
  return rsdp;
}

const struct acpi_madt *get_acpi_madt(void) {
  return madt;
}

static void init_root_tables(void) {
  rsdp = (struct acpi_rsdp*)
    memmem((void*)EBDA_ADDR, EBDA_NEXT_ADDR - EBDA_ADDR, RSDP_SIGNATURE, 8);
  if (!rsdp)
    rsdp = (struct acpi_rsdp*)
      memmem((void*)BIOS_ROM_ADDR, BIOS_ROM_NEXT_ADDR - BIOS_ROM_ADDR,
             RSDP_SIGNATURE, 8);

  if (rsdp) { // make sure RSDT and XSDT are mapped
    if (!get_page_mapping(rsdp->rsdt_addr, NULL, NULL, NULL))
      map_page(rsdp->rsdt_addr, rsdp->rsdt_addr, 0, 0);
    if (!get_page_mapping(rsdp->xsdt_addr, NULL, NULL, NULL))
      map_page(rsdp->xsdt_addr, rsdp->xsdt_addr, 0, 0);
  }

  if (rsdp->rsdt_addr) { // make sure all tables pointed by RSDT are mapped
    const struct acpi_rsdt *rsdt = get_acpi_rsdt();
    int num = (rsdt->header.length - sizeof(rsdt->header)) >> 2;
    for (int i = 0; i < num; i++)
      if (!get_page_mapping(rsdt->tables[i], NULL, NULL, NULL))
        map_page(rsdt->tables[i], rsdt->tables[i], 0, 0);
  }

  if (rsdp->xsdt_addr) { // make sure all tables pointed by XSDT are mapped
    const struct acpi_xsdt *xsdt = get_acpi_xsdt();
    int num = (xsdt->header.length - sizeof(xsdt->header)) >> 4;
    for (int i = 0; i < num; i++)
      if (!get_page_mapping(xsdt->tables[i], NULL, NULL, NULL))
        map_page(xsdt->tables[i], xsdt->tables[i], 0, 0);
  }
}

static void *find_psdt_table(char signature[4]) {
  const struct acpi_rsdt *rsdt = get_acpi_rsdt();
  if (!rsdt)
    return NULL;

  int num = (rsdt->header.length - sizeof(rsdt->header)) >> 2;
  for (int i = 0; i < num; i++)
    if (!memcmp((void*)(uint64_t)rsdt->tables[i], signature, 4))
      return (void*)(uint64_t)rsdt->tables[i];

  return NULL;
}

const struct acpi_madt_entry_header*
next_acpi_madt_entry(const struct acpi_madt_entry_header *entry) {
  const struct acpi_madt *madt = get_acpi_madt();
  if (!madt)
    return NULL;

  if (!entry)
    return (struct acpi_madt_entry_header*)(madt + 1);

  uint64_t eaddr = (uint64_t)entry + entry->length;
  if (eaddr - (uint64_t)madt >= madt->header.length)
    return NULL;

  return (struct acpi_madt_entry_header*)eaddr;
}

void init_acpi(void) {
  init_root_tables();
  madt = (struct acpi_madt*)find_psdt_table(MADT_SIGNATURE);
}
