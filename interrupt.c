#include "desc_table.h"
#include "display.h"
#include "interrupt.h"

static void *get_default_isr(void) {
  ISR_PROLOG();
  printf("#INT(ss: %d, rsp: %d, rflags: %d, cs: %d, rip: %d)\n",
         stack_frame->ss, stack_frame->rsp, stack_frame->rflags,
         stack_frame->cs, stack_frame->rip);
  ISR_EPILOG();
}

static void *get_default_eisr(void) {
  ISR_ERR_PROLOG();
  printf("#INT(ss: %d, rsp: %d, rflags: %d, cs: %d, rip: %d, err_code: %d)\n",
         stack_frame->ss, stack_frame->rsp, stack_frame->rflags,
         stack_frame->cs, stack_frame->rip, stack_frame->error_code);
  for(;;);
  ISR_ERR_EPILOG();
}

void init_interrupts(void) {
  for (int i = 0; i < INT_VECTOR_NUMBER; i++)
    if (!is_int_reserved(i))
      set_isr(i, is_int_error(i) ? get_default_eisr() : get_default_isr());
}
