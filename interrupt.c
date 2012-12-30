#include "display.h"
#include "interrupt.h"

static void *default_int_handler(void) {
  INTR_HANDLER_PROLOG();
  printf("Default interrupt handler called...\n");
  INTR_HANDLER_EPILOG();
}

static void *default_int_err_handler(void) {
  INTR_ERR_HANDLER_PROLOG();
  printf("Default interrupt handler called (error_code=%d)...\n", error_code);
  INTR_ERR_HANDLER_EPILOG();
}

static struct idt_desc {
  

} idt[256];

void init_interrupts(void) {

}
