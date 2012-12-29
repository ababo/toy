#include "display.h"
#include "interrupt.h"

static void default_int_handler (void) {
  printf("Default interrupt handler called...\n");
}

void init_interrupts (void) {

}

intr_handler get_intr_handler (int intr) {

}

void set_intr_handler (int intr, intr_handler handler) {

}
