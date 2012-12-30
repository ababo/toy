#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "util.h"

#define INTR_HANDLER_PUSHA()                                            \
  __asm__("push %rax\npush %rbx\npush %rcx\npush %rdx");                \
  __asm__("push %rsi\npush %rdi\npush %r8\npush %r9\npush %r10");       \
  __asm__("push %r11\npush %r12\npush %r13\npush %r14\npush %r15");

#define INTR_HANDLER_POPA()                                    \
  __asm__("pop %r15\npop %r14\npop %r13\npop %r12\npop %r11"); \
  __asm__("pop %r10\npop %r9\npop %r8\npop %rdi\npop %rsi");   \
  __asm__("pop %rdx\npop %rcx\npop %rbx\npop %rax");

#define INTR_HANDLER_PROLOG()                                          \
  __asm__ goto("jmp %l[isr_end]\n.align 16" : : : "memory" : isr_end); \
isr_begin:                                                             \
  INTR_HANDLER_PUSHA();

#define INTR_ERR_HANDLER_PROLOG()                                      \
  __asm__ goto("jmp %l[isr_end]\n.align 16" : : : "memory" : isr_end); \
isr_begin:                                                             \
  INTR_HANDLER_PUSHA();                                                \
  int error_code;                                                      \
  __asm__("movl (%%rsp), %0" : "=a"(error_code));

#define INTR_HANDLER_EPILOG()                                           \
  INTR_HANDLER_POPA();                                                  \
  __asm__("iret");                                                      \
isr_end:                                                                \
  __asm__ goto("movq %l[isr_begin], %%rax" : : : "memory" : isr_begin);

#define INTR_ERR_HANDLER_EPILOG()                                       \
  INTR_HANDLER_POPA();                                                  \
  __asm__("subq $-8, %rsp\niret");                                      \
isr_end:                                                                \
  __asm__ goto("movq %l[isr_begin], %%rax" : : : "memory" : isr_begin);

typedef void *(*intr_handler)(void);

void init_interrupts(void);

#endif // INTERRUPT_H
