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

ASM(".text; .global __kstart;"
    "__kstart: movw $" STR_EXPAND(SEGMENT_DATA) ", %ax;"
    "movw %ax, %ds; movw %ax, %ss;"
    "movq $(__bsp_boot_stack + "
      STR_EXPAND(CONFIG_BSP_BOOT_STACK_SIZE) "), %rsp;"
    "call boot64;"
    "jmp __halt");

// ap cpu trampoline
ASM(".text; .code16; .global __bstart16;"
    "__bstart16: inb $0x92, %al; orb $2, %al; outb %al, $0x92;" // enable A20
    "movb $0xFF, %al; outb %al, $0xA1; outb %al, $0x21;" // disable IRQs
    "movl $" STR_EXPAND(CR4_PAE) ", %eax; movl %eax, %cr4;"
    "movl $__page_map, %eax; movl %eax, %cr3;"
    "movl $" STR_EXPAND(MSR_EFER) ", %ecx; rdmsr; orl $"
      STR_EXPAND(MSR_EFER_LME) ", %eax; wrmsr;"
    "movl %cr0, %eax; orl $" STR_EXPAND(CR0_PE | CR0_PG)
      ", %eax; movl %eax, %cr0;"
    "lgdt (" STR_EXPAND(BSTART16_ADDR) " + gdti - __bstart16);"
    "ljmpl $" STR_EXPAND(SEGMENT_CODE) ", $__kstart_ap;"
    "gdti: .word 3 * 8 - 1; .long __gdt;"
    ".global __bstart16_end; __bstart16_end:; .code64");

ASM(".text; .global __kstart_ap;"
    "__kstart_ap: movw $" STR_EXPAND(SEGMENT_DATA) ", %ax;"
    "movw %ax, %ds; movw %ax, %ss;"
    "movq (ap_boot_stack), %rsp;"
    "addq $" STR_EXPAND(CONFIG_AP_BOOT_STACK_SIZE) ", %rsp;"
    "movq %cr4, %rax;" // enable SSE
    "orl $" STR_EXPAND(CR4_OSFXSR) ", %eax;"
    "movq %rax, %cr4;"
    "call boot64_ap;"
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
