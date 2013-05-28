#include "../schedule.h"
#include "cpu.h"

ASM(".global __exit_thread\n__exit_thread:\n"
    "pushq %rax\nmovl $" STR_EXPAND(MSR_FS_BASE) ", %ecx\n"
    "rdmsr\nmovl %edx, %edi\nshll $32, %edi\nmovl %eax, %edi\n"
    "popq %rsi\ncallq stop_thread");

err_code set_thread_context(struct thread_data *thread, thread_proc proc,
                            uint64_t input) {

}

void __issue_task_interrupt(int cpu) {

}
