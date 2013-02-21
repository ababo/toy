#include "interrupt.h"

ISR_IMPL(default) {
  printf("#%s (ss: %X, rsp: %X, rflags: %X, cs: %X, rip: %X", data << 8 >> 8,
         stack_frame->ss, stack_frame->rsp, stack_frame->rflags,
         stack_frame->cs, stack_frame->rip);
  if (is_int_error(data >> 56))
    printf(", error_code: %X)\n", stack_frame->error_code);
  else
    printf(")\n");
  asm("hlt");
}

#define ISR_SIMPLE_GETTER(mnemonic)                                     \
  ISR_GETTER(mnemonic, default,                                         \
             ((uint64_t)INT_VECTOR_##mnemonic << 56) + #mnemonic)

ISR_SIMPLE_GETTER(DE);
ISR_SIMPLE_GETTER(NMI);
ISR_SIMPLE_GETTER(BP);
ISR_SIMPLE_GETTER(OF);
ISR_SIMPLE_GETTER(BR);
ISR_SIMPLE_GETTER(UD);
ISR_SIMPLE_GETTER(NM);
ISR_SIMPLE_GETTER(DF)
ISR_SIMPLE_GETTER(TS);
ISR_SIMPLE_GETTER(NP);
ISR_SIMPLE_GETTER(SS);
ISR_SIMPLE_GETTER(GP);
ISR_SIMPLE_GETTER(PF);
ISR_SIMPLE_GETTER(MF);
ISR_SIMPLE_GETTER(AC);
ISR_SIMPLE_GETTER(MC);
ISR_SIMPLE_GETTER(XM);

void init_interrupts(void) {
  set_isr(INT_VECTOR_DE, DE_isr_getter());
  set_isr(INT_VECTOR_NMI, NMI_isr_getter());
  set_isr(INT_VECTOR_BP, BP_isr_getter());
  set_isr(INT_VECTOR_OF, OF_isr_getter());
  set_isr(INT_VECTOR_BR, BR_isr_getter());
  set_isr(INT_VECTOR_UD, UD_isr_getter());
  set_isr(INT_VECTOR_NM, NM_isr_getter());
  set_isr(INT_VECTOR_DF, DF_isr_getter());
  set_isr(INT_VECTOR_TS, TS_isr_getter());
  set_isr(INT_VECTOR_NP, NP_isr_getter());
  set_isr(INT_VECTOR_SS, SS_isr_getter());
  set_isr(INT_VECTOR_GP, GP_isr_getter());
  set_isr(INT_VECTOR_PF, PF_isr_getter());
  set_isr(INT_VECTOR_MF, MF_isr_getter());
  set_isr(INT_VECTOR_AC, AC_isr_getter());
  set_isr(INT_VECTOR_MC, MC_isr_getter());
  set_isr(INT_VECTOR_XM, XM_isr_getter());
  LOG_DEBUG("init_interrupts: done");
}
