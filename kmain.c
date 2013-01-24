#include "apic.h"
#include "desc_table.h"
#include "display.h"
#include "interrupt.h"
#include "page_table.h"
#include "util.h"

void kmain(void) {
  set_frame(0, 0, ROW_NUMBER, COL_NUMBER, COLOR_WHITE, COLOR_LOW_BLUE);
  clear_frame();
  set_cursor(true);
  printf("Display initialized...\n");
  init_desc_tables();
  printf("Descriptor tables initialized...\n");
  init_interrupts();
  printf("Interrupts initialized...\n");
  init_page_tables();
  printf("Page tables initialized...\n");
  init_apic();
  printf("APIC initialized...\n");
};
