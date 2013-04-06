#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "util.h"

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
  uint64_t r15, r14, r13, r12, r11, r10, r9, r8, rdi, rsi, rdx, rcx, rbx, rax;
  uint8_t fxdata[512];
  uint32_t error_code;
  uint64_t rip;
  uint16_t cs;
  uint64_t rflags, rsp;
  uint16_t ss;
};

#define ISR_IMPL(name)                                                  \
  static NOINLINE                                                       \
  void name##_isr_impl(UNUSED struct int_stack_frame *stack_frame,      \
                       UNUSED uint64_t data)

#define ISR_GETTER(name, impl_name, data)                            \
  static NOINLINE void *name##_isr_getter(void) {                    \
    ASMV("jmp 2f\n.align 16\n1: andq $(~0xF), %rsp");                \
    ASMV("subq $512, %rsp\nfxsave (%rsp)");                          \
    ASMV("push %rax\npush %rbx\npush %rcx\npush %rdx");              \
    ASMV("push %rsi\npush %rdi\npush %r8\npush %r9\npush %r10");     \
    ASMV("push %r11\npush %r12\npush %r13\npush %r14\npush %r15");   \
    ASMV("movq %%rsp, %%rdi\nmovabsq $%P0, %%rsi" : : "i"(data));    \
    ASMV("callq %P0" : : "i"(impl_name##_isr_impl));                 \
    ASMV("pop %r15\npop %r14\npop %r13\npop %r12\npop %r11");        \
    ASMV("pop %r10\npop %r9\npop %r8\npop %rdi\npop %rsi");          \
    ASMV("pop %rdx\npop %rcx\npop %rbx\npop %rax");                  \
    ASMV("fxrstor (%rsp)\naddq $(512 + 8), %rsp");                   \
    void *isr;                                                       \
    ASMV("iretq\n2: movq $1b, %0" : "=m"(isr));                      \
    return isr;                                                      \
  }

#define ISR_DEFINE(name, data)                  \
  ISR_IMPL(name);                               \
  ISR_GETTER(name, name, data)                  \
  ISR_IMPL(name)

void dump_int_stack_frame(const struct int_stack_frame *stack_frame);

void *get_isr(int vector);
void set_isr(int vector, void *isr);

void init_interrupts(void);

#endif // INTERRUPT_H
