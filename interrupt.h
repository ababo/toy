#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "util.h"

#define INT_VECTOR_NUMBER 34

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

#define INT_VECTOR_SPURIOUS 32
#define INT_VECTOR_TIMER 33

static inline bool is_int_reserved(int vector) {
  return vector == 15 || (vector >= 20 && vector <= 31);
}
static inline bool is_int_error(int vector) {
  return vector == 8 || (vector >= 10 && vector <= 14) || vector == 17;
}

struct int_stack_frame {
  uint32_t error_code;
  uint64_t rip;
  uint16_t cs;
  uint64_t rflags;
  uint64_t rsp;
  uint16_t ss;
};

#define ISR_DECLARATION(name)                                           \
  static __attribute__ ((noinline)) void name(struct int_stack_frame    \
                                              *stack_frame)

#define ISR_CONTAINER(name)                                             \
  ISR_DECLARATION(name##_isr);                                          \
  static __attribute__ ((noinline)) void *get_##name##_isr(void) {      \
  __asm__("jmp 2f\n.align 16\n1:andq $(~0b1111), %rsp");                \
  __asm__("subq $512, %rsp\nfxsave (%rsp)");                            \
  __asm__("push %rax\npush %rbx\npush %rcx\npush %rdx");                \
  __asm__("push %rsi\npush %rdi\npush %r8\npush %r9\npush %r10");       \
  __asm__("push %r11\npush %r12\npush %r13\npush %r14\npush %r15");     \
  __asm__("leaq (512 + 14 * 8)(%rsp), %rdi");                           \
  __asm__("callq %P0" : : "i"(name##_isr));                             \
  __asm__("pop %r15\npop %r14\npop %r13\npop %r12\npop %r11");          \
  __asm__("pop %r10\npop %r9\npop %r8\npop %rdi\npop %rsi");            \
  __asm__("pop %rdx\npop %rcx\npop %rbx\npop %rax");                    \
  __asm__("fxrstor (%rsp)\naddq $(512 + 8), %rsp");                     \
  __asm__("iretq\n2:\nmovq $1b, %rax");                                 \
  }                                                                     \
  ISR_DECLARATION(name##_isr)

void init_interrupts(void);

void *get_isr(int vector);
void set_isr(int vector, void *isr);

#endif // INTERRUPT_H
