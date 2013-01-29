#include "display.h"
#include "interrupt.h"

static void print_int(const char *mnemonic,
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

#define SIMPLE_ISR(mnemonic)                                            \
  ISR_CONTAINER(mnemonic) {                                             \
    print_int("#" #mnemonic, stack_frame,                               \
              is_int_error(INT_VECTOR_##mnemonic));                     \
}

SIMPLE_ISR(DE);
SIMPLE_ISR(NMI);
SIMPLE_ISR(BP);
SIMPLE_ISR(OF);
SIMPLE_ISR(BR);
SIMPLE_ISR(UD);
SIMPLE_ISR(NM);
SIMPLE_ISR(DF)
SIMPLE_ISR(TS);
SIMPLE_ISR(NP);
SIMPLE_ISR(SS);
SIMPLE_ISR(GP);
SIMPLE_ISR(PF);
SIMPLE_ISR(MF);
SIMPLE_ISR(AC);
SIMPLE_ISR(MC);
SIMPLE_ISR(XM);

void init_interrupts(void) {
  set_isr(INT_VECTOR_DE, get_DE_isr());
  set_isr(INT_VECTOR_NMI, get_NMI_isr());
  set_isr(INT_VECTOR_BP, get_BP_isr());
  set_isr(INT_VECTOR_OF, get_OF_isr());
  set_isr(INT_VECTOR_BR, get_BR_isr());
  set_isr(INT_VECTOR_UD, get_UD_isr());
  set_isr(INT_VECTOR_NM, get_NM_isr());
  set_isr(INT_VECTOR_DF, get_DF_isr());
  set_isr(INT_VECTOR_TS, get_TS_isr());
  set_isr(INT_VECTOR_NP, get_NP_isr());
  set_isr(INT_VECTOR_SS, get_SS_isr());
  set_isr(INT_VECTOR_GP, get_GP_isr());
  set_isr(INT_VECTOR_PF, get_PF_isr());
  set_isr(INT_VECTOR_MF, get_MF_isr());
  set_isr(INT_VECTOR_AC, get_AC_isr());
  set_isr(INT_VECTOR_MC, get_MC_isr());
  set_isr(INT_VECTOR_XM, get_XM_isr());
}
