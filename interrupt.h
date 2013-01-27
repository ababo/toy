#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "util.h"

#define INT_VECTOR_NUMBER 34

#define INT_VECTOR_PF 14
#define INT_VECTOR_SPURIOUS 32
#define INT_VECTOR_TIMER 33

static inline bool is_int_reserved(int vector) {
  return vector == 15 || (vector >= 20 && vector <= 31);
}
static inline bool is_int_error(int vector) {
  return vector == 8 || (vector >= 10 && vector <= 14) || vector == 17;
}

#define ISR_PUSHA()                                                     \
  __asm__("push %rbp\npush %rax\npush %rbx\npush %rcx\npush %rdx");     \
  __asm__("push %rsi\npush %rdi\npush %r8\npush %r9\npush %r10");       \
  __asm__("push %r11\npush %r12\npush %r13\npush %r14\npush %r15");

#define ISR_POPA()                                             \
  __asm__("pop %r15\npop %r14\npop %r13\npop %r12\npop %r11"); \
  __asm__("pop %r10\npop %r9\npop %r8\npop %rdi\npop %rsi");   \
  __asm__("pop %rdx\npop %rcx\npop %rbx\npop %rax\npop %rbp");

struct int_stack_frame {
  uint32_t error_code;
  uint64_t rip;
  uint16_t cs;
  uint64_t rflags;
  uint64_t rsp;
  uint16_t ss;
};

#define ISR_PROLOG()                                                   \
  struct int_stack_frame *stack_frame;                                 \
  __asm__("jmp 2f\n.align 16\n1:");                                    \
  ISR_PUSHA();                                                         \
  __asm__("movq %rsp, %rbp");                                          \
  __asm__("leaq (15 * 8 - 8)(%%rsp), %0" : "=a"(stack_frame));

#define ISR_EPILOG()                                                    \
  ISR_POPA();                                                           \
  __asm__("iretq\n2:\nmovq $1b, %rax");

#define ISR_ERR_PROLOG()                                               \
  struct int_stack_frame *stack_frame;                                 \
  __asm__("jmp 2f\n.align 16\n1:");                                    \
  ISR_PUSHA();                                                         \
  __asm__("movq %rsp, %rbp");                                          \
  __asm__("leaq (15 * 8)(%%rsp), %0" : "=a"(stack_frame));

#define ISR_ERR_EPILOG()                                                \
  ISR_POPA();                                                           \
  __asm__("addq $8, %rsp\niretq\n2:\nmovq $1b, %rax");

#define ISR_CONTAINER(name) static __attribute__ ((noinline)) void *name(void)

void init_interrupts(void);

void *get_isr(int vector);
void set_isr(int vector, void *isr);

#endif // INTERRUPT_H
