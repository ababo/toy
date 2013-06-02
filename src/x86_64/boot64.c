#include "../common.h"
#include "../config.h"
#include "../cpu_info.h"
#include "../memory.h"
#include "../schedule.h"
#include "../sync.h"
#include "acpi.h"
#include "apic.h"
#include "cpu.h"
#include "interrupt.h"
#include "pci.h"
#include "vga.h"

#define BSTART16_ADDR 0x1000

ASM(".text\n.global __kstart\n"
    "__kstart: movw $" STR_EXPAND(SEGMENT_DATA) ", %ax\n"
    "movw %ax, %ds\nmovw %ax, %ss\n"
    "movq $(__bsp_boot_stack + "
      STR_EXPAND(CONFIG_BSP_BOOT_STACK_SIZE) "), %rsp\n"
    "call boot64\n"
    "jmp __halt");

// ap cpu trampoline
ASM(".text\n.code16\n.global __bstart16\n"
    "__bstart16: inb $0x92, %al\norb $2, %al\noutb %al, $0x92\n" // enable A20
    "movb $0xFF, %al\noutb %al, $0xA1\noutb %al, $0x21\n" // disable IRQs
    "movl $" STR_EXPAND(CR4_PAE) ", %eax\nmovl %eax, %cr4\n"
    "movl $__page_map, %eax\nmovl %eax, %cr3\n"
    "movl $" STR_EXPAND(MSR_EFER) ", %ecx\nrdmsr\norl $"
      STR_EXPAND(MSR_EFER_LME) ", %eax\nwrmsr\n"
    "movl %cr0, %eax\norl $" STR_EXPAND(CR0_PE | CR0_PG)
      ", %eax\nmovl %eax, %cr0\n"
    "lgdt (" STR_EXPAND(BSTART16_ADDR) " + gdti - __bstart16)\n"
    "ljmpl $" STR_EXPAND(SEGMENT_CODE) ", $__kstart_ap\n"
    "gdti: .word 3 * 8 - 1\n.long __gdt\n"
    ".global __bstart16_end\n__bstart16_end:\n.code64");

ASM(".text\n.global __kstart_ap\n"
    "__kstart_ap: movw $" STR_EXPAND(SEGMENT_DATA) ", %ax\n"
    "movw %ax, %ds\nmovw %ax, %ss\n"
    "movq (ap_boot_stack), %rsp\n"
    "addq $" STR_EXPAND(CONFIG_AP_BOOT_STACK_SIZE) ", %rsp\n"
    "movq %cr4, %rax\n" // enable SSE
    "orl $" STR_EXPAND(CR4_OSFXSR) ", %eax\n"
    "movq %rax, %cr4\n"
    "call boot64_ap\n"
    "jmp __halt");

static uint8_t *ap_boot_stack = NULL;
static int started_cpus = 1;

static void start_ap_cpus(void) {
  if (get_cpus() == 1)
    return;

  extern uint8_t __bstart16, __bstart16_end;
  memcpy((void*)BSTART16_ADDR, &__bstart16, &__bstart16_end - &__bstart16);

  ap_boot_stack = kmalloc(CONFIG_AP_BOOT_STACK_SIZE);
  if (!ap_boot_stack)
    PANIC("failed to allocate memory");

  for (int i = 0; i < get_cpus(); i++)
    if (i != get_bsp_cpu() && !start_ap_cpu(i, BSTART16_ADDR, &started_cpus))
      PANIC("failed to start AP CPU %d", i);

  kfree(ap_boot_stack);
}

#define KMAIN_THREAD_PRIORITY 3

extern uint64_t kmain(uint64_t);

static void start_kmain_thread(void) {
  struct thread_data *thrd = kmalloc(sizeof(*thrd));
  if (!thrd)
    PANIC("failed to allocate memory");

  extern uint8_t __bsp_boot_stack;
  memset(thrd, 0, sizeof(*thrd));
  thrd->stack = &__bsp_boot_stack;
  thrd->stack_size = CONFIG_BSP_BOOT_STACK_SIZE;
  BIT_ARRAY_SET(thrd->affinity, get_bsp_cpu());
  thrd->priority = KMAIN_THREAD_PRIORITY;
  thrd->fixed_priority = true;

  thread_id id;
  if (set_thread_context(thrd, kmain, 0) ||
      attach_thread(thrd, &id) || resume_thread(id))
    PANIC("failed to start kinit_thread");

  ASMV("jmp __halt");
}

void boot64(void) {
  init_vga();
  init_acpi();
  init_cpu_info();
  init_memory();
  init_interrupts();
  init_apic();
  init_pci();
  ASMV("sti");
  start_ap_cpus();
  init_scheduler();
  init_sync();
  start_kmain_thread();
}

void boot64_ap(void) {
  init_interrupts();
  init_apic();
  ASMV("sti");
  init_scheduler();
  started_cpus++;
}
