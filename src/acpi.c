#include "acpi.h"
#include "addr_map.h"
#include "page_map.h"

#define RSDP_SIGNATURE "RSD PTR "
#define RSDT_SIGNATURE "RSDT"
#define XSDT_SIGNATURE "XSDT"
#define MADT_SIGNATURE "APIC"
#define SRAT_SIGNATURE "SRAT"

static const struct acpi_rsdp *rsdp;
static const struct acpi_madt *madt;
static const struct acpi_srat *srat;

const struct acpi_rsdp *get_acpi_rsdp(void) {
  return rsdp;
}

const struct acpi_madt *get_acpi_madt(void) {
  return madt;
}

const struct acpi_srat *get_acpi_srat(void) {
  return srat;
}

static void init_root_tables(void) {
  rsdp = (struct acpi_rsdp*)
    memmem((void*)ADDR_EBDA, ADDR_AFTER_EBDA - ADDR_EBDA, RSDP_SIGNATURE, 8);
  if (!rsdp)
    rsdp = (struct acpi_rsdp*)
      memmem((void*)ADDR_BIOS_ROM, ADDR_AFTER_BIOS_ROM - ADDR_BIOS_ROM,
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

bool get_next_acpi_entry(const void *table, void *entry_pptr, int type) {
  if (!table || !entry_pptr)
    return false;

  int tsize;
  if (table == get_acpi_madt())
    tsize = sizeof(struct acpi_madt);
  else if (table == get_acpi_srat())
    tsize = sizeof(struct acpi_srat);
  else
    return false;

  const struct acpi_entry_header **entry = entry_pptr;
  if (*entry)
    *entry = (void*)((uint64_t)*entry + (*entry)->length);
  else
    *entry = (void*)((uint64_t)table + tsize);

  int length = ((struct acpi_header*)table)->length;
  while ((uint64_t)*entry - (uint64_t)table < length)
    if (type == -1 || (*entry)->type == type)
      return true;
    else
      *entry = (void*)((uint64_t)*entry + (*entry)->length);

  return false;
}

void init_acpi(void) {
  init_root_tables();
  madt = find_psdt_table(MADT_SIGNATURE);
  srat = find_psdt_table(SRAT_SIGNATURE);
  LOG_DEBUG("RSDP: %X, RSDT: %X, XSDT: %X, MADT: %X, SRAT: %X",
            get_acpi_rsdp(), get_acpi_rsdt(), (uint64_t)get_acpi_xsdt() >> 32,
            get_acpi_madt(), get_acpi_srat());
}
