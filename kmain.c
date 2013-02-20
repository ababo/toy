#include "acpi.h"
#include "apic.h"
#include "desc_table.h"
#include "display.h"
#include "interrupt.h"
#include "mem_map.h"
#include "page_map.h"
#include "util.h"

ISR_DEFINE(test, 0) {
  printf("#DE: jumping to next instruction.\n");
  stack_frame->rip += 2;
}

void kmain(void) {
  set_frame(0, 0, ROW_NUMBER, COL_NUMBER, COLOR_WHITE, COLOR_LOW_BLUE);
  clear_frame();
  set_cursor(true);
  printf("Display initialized.\n");
  init_desc_tables();
  printf("Descriptor tables initialized.\n");
  init_interrupts();
  printf("Interrupts initialized.\n");
  printf("Trying to initiate #DE.\n");
  set_isr(INT_VECTOR_DE, test_isr_getter());
  asm("div %P0" : : "a"(0));
  printf("Returned from #DE.\n");
  printf("Usable memory size: %d MiB.\n",
         ROUND_DIV(get_usable_memory_size(), 0x100000));
  init_page_map(PAGE_MAP_ADDR, 256 * (1L << 30));
  printf("Page map initialized.\n");
  init_apic();
  printf("APIC initialized.\n");
  init_acpi();
  printf("ACPI initialized.\n");

  const struct acpi_madt_entry_header *h = 0;
  while ((h = next_acpi_madt_entry(h)))
    if (h->type == ACPI_MADT_LAPIC_TYPE) {
      const struct acpi_madt_lapic_entry *e =
        (const struct acpi_madt_lapic_entry*)h;
      printf("CPU core detected (acpi_cpuid: %X, apic_id: %X, usable: %s)\n",
             e->acpi_cpuid, e->apic_id, bool_str(e->flags & 1));
    }
}
