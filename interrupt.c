#include "display.h"
#include "interrupt.h"

static inline void print_int(const char *mnemonic,
                             const struct int_stack_frame *stack_frame,
                             bool error_code) {
  printf("%s (ss: %X, rsp: %X, rflags: %X, cs: %X, rip: %X", mnemonic,
         stack_frame->ss, stack_frame->rsp, stack_frame->rflags,
         stack_frame->cs, stack_frame->rip);
  if (error_code)
    printf(", error_code: %X)\n", stack_frame->error_code);
  else
    printf(")\n");
}

static void *get_pf_isr(void) {
  ISR_ERR_PROLOG();
  print_int("#PF", stack_frame, true);
  ISR_ERR_EPILOG();
}

static void *get_default_isr(void) {
  ISR_PROLOG();
  print_int("#?", stack_frame, false);
  ISR_EPILOG();
}

static void *get_default_err_isr(void) {
  ISR_ERR_PROLOG();
  print_int("#?", stack_frame, true);
  ISR_ERR_EPILOG();
}

void init_interrupts(void) {
  for (int i = 0; i < INT_VECTOR_NUMBER; i++)
    if (!is_int_reserved(i))
      set_isr(i, is_int_error(i) ? get_default_err_isr() : get_default_isr());

  set_isr(INT_VECTOR_PF, get_pf_isr());
}
