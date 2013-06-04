#ifndef __X86_64_INTERRUPT_H
#define __X86_64_INTERRUPT_H

#include "../common.h"

#define INT_VECTORS 35

#define INT_VECTOR_DE 0 // divide error
#define INT_VECTOR_NMI 2 // non-maskable interrupt
#define INT_VECTOR_BP 3 // breakpoint
#define INT_VECTOR_OF 4 // overflow
#define INT_VECTOR_BR 5 // BOUND range exceeded
#define INT_VECTOR_UD 6 // undefined opcode
#define INT_VECTOR_NM 7 // device not available
#define INT_VECTOR_DF 8 // double fault
#define INT_VECTOR_TS 10 // invalid TSS
#define INT_VECTOR_NP 11 // segment not present
#define INT_VECTOR_SS 12 // stack-segment fault
#define INT_VECTOR_GP 13 // general protection
#define INT_VECTOR_PF 14 // page fault
#define INT_VECTOR_MF 16 // math fault
#define INT_VECTOR_AC 17 // alignment check
#define INT_VECTOR_MC 18 // machine check
#define INT_VECTOR_XM 19 // SIMD floating-point exception

#define INT_VECTOR_APIC_SPURIOUS 32
#define INT_VECTOR_APIC_TIMER 33
#define INT_VECTOR_SCHEDULER_TASK 34

static inline bool is_int_reserved(int vector) {
  return vector == 15 || (vector >= 20 && vector <= 31);
}
static inline bool is_int_error(int vector) {
  return vector == 8 || (vector >= 10 && vector <= 14) || vector == 17;
}

struct int_stack_frame {
  uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
  uint64_t rdi, rsi, rbp, rdx, rcx, rbx, rax;
  uint8_t fxdata[512];
  uint32_t error_code;
  uint64_t rip;
  uint16_t cs;
  uint64_t rflags, rsp;
  uint16_t ss;
};

#define DEFINE_INT_HANDLER(name)                                        \
  NOINLINE void __handle_##name##_int(                                  \
      UNUSED struct int_stack_frame *stack_frame, UNUSED uint64_t data)

#define DEFINE_ISR_WRAPPER(name, handler_name, data)                 \
  NOINLINE void *__get_##name##_isr(void) {                          \
    ASMV("jmp 2f; .align 16; 1: andq $(~0xF), %rsp");                \
    ASMV("subq $512, %rsp; fxsave (%rsp)");                          \
    ASMV("push %rax; push %rbx; push %rcx; push %rdx; push %rbp");   \
    ASMV("push %rsi; push %rdi; push %r8; push %r9; push %r10");     \
    ASMV("push %r11; push %r12; push %r13; push %r14; push %r15");   \
    ASMV("movq %%rsp, %%rdi; movabsq $%P0, %%rsi" : : "i"(data));    \
    ASMV("callq %P0" : : "i"(__handle_##handler_name##_int));        \
    ASMV("pop %r15; pop %r14; pop %r13; pop %r12; pop %r11");        \
    ASMV("pop %r10; pop %r9; pop %r8; pop %rdi; pop %rsi");          \
    ASMV("pop %rbp; pop %rdx; pop %rcx; pop %rbx; pop %rax");        \
    ASMV("fxrstor (%rsp); addq $(512 + 8), %rsp");                   \
    void *isr;                                                       \
    ASMV("iretq; 2: movq $1b, %0" : "=m"(isr));                      \
    return isr;                                                      \
  }

#define DEFINE_ISR(name, data)             \
  DEFINE_INT_HANDLER(name);                \
  DEFINE_ISR_WRAPPER(name, name, data)     \
  DEFINE_INT_HANDLER(name)

void dump_int_stack_frame(const struct int_stack_frame *stack_frame);

void *get_isr(int vector);
void set_isr(int vector, void *isr);

void init_interrupts(void);

#endif // __X86_64_INTERRUPT_H
