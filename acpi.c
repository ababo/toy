#include "acpi.h"
#include "mem_map.h"
#include "page_map.h"

#define RSDP_SIGNATURE "RSD PTR "

static const struct acpi_rsdp *rsdp;

const struct acpi_rsdp *get_acpi_rsdp(void) {
  return rsdp;
}

void init_acpi(void) {
  rsdp = (struct acpi_rsdp*)
    memmem((void*)EBDA_ADDR, EBDA_NEXT_ADDR - EBDA_ADDR, RSDP_SIGNATURE, 8);
  if (!rsdp)
    rsdp = (struct acpi_rsdp*)
      memmem((void*)BIOS_ROM_ADDR, BIOS_ROM_NEXT_ADDR - BIOS_ROM_ADDR,
             RSDP_SIGNATURE, 8);

  if (rsdp) {
    if (!get_page_mapping(rsdp->rsdt_addr, NULL, NULL, NULL))
      map_page(rsdp->rsdt_addr, rsdp->rsdt_addr, 0, 0);
    if (!get_page_mapping(rsdp->xsdt_addr, NULL, NULL, NULL))
      map_page(rsdp->xsdt_addr, rsdp->xsdt_addr, 0, 0);
  }
}
