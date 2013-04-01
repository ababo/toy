#include "acpi.h"
#include "apic.h"
#include "boot.h"
#include "config.h"
#include "cpu_info.h"
#include "interrupt.h"
#include "memory.h"
#include "util.h"
#include "schedule.h"
#include "vga.h"

ASM(".text\n.global kstart\n"
    "kstart: movw $" STR_EXPAND(SEGMENT_DATA) ", %ax\n"
    "movw %ax, %ds\nmovw %ax, %ss\n"
    "movq $(bsp_boot_stack + "
      STR_EXPAND(CONFIG_BSP_BOOT_STACK_SIZE) "), %rsp\n"
    "call kmain\n"
    "halt: hlt\njmp halt");

#define BSTART16_ADDR 0x1000

// ap cpu trampoline
ASM(".text\n.code16\n.global bstart16\n"
    "bstart16: inb $0x92, %al\norb $2, %al\noutb %al, $0x92\n" // enable A20
    "movb $0xFF, %al\noutb %al, $0xA1\noutb %al, $0x21\n" // disable IRQs
    "movl $" STR_EXPAND(CR4_PAE) ", %eax\nmovl %eax, %cr4\n"
    "movl $page_map, %eax\nmovl %eax, %cr3\n"
    "movl $" STR_EXPAND(MSR_EFER) ", %ecx\nrdmsr\norl $"
      STR_EXPAND(MSR_EFER_LME) ", %eax\nwrmsr\n"
    "movl %cr0, %eax\norl $" STR_EXPAND(CR0_PE | CR0_PG)
      ", %eax\nmovl %eax, %cr0\n"
    "lgdt (" STR_EXPAND(BSTART16_ADDR) " + gdti - bstart16)\n"
    "ljmpl $" STR_EXPAND(SEGMENT_CODE) ", $kstart_ap\n"
    "gdti: .word 3 * 8 - 1\n.long gdt\n"
    ".global bstart16_end\nbstart16_end:\n.code64");

uint8_t *ap_boot_stack = NULL;
int started_cpus = 1;

static void start_ap_cpus(void) {
  if (get_cpus() == 1)
    return;

  extern uint8_t bstart16, bstart16_end;
  memcpy((void*)BSTART16_ADDR, &bstart16, &bstart16_end - &bstart16);

  ap_boot_stack = kmalloc(CONFIG_AP_BOOT_STACK_SIZE);
  if (!ap_boot_stack) {
    LOG_ERROR("failed to allocate memory");
    ASMV("hlt");
  }

  for (int i = 0; i < get_cpus(); i++)
    if (i != get_bsp_cpu() &&
        !start_ap_cpu(get_cpu_desc(i)->apic_id, BSTART16_ADDR, &started_cpus))
      LOG_ERROR("failed to start AP CPU %d", i);

  kfree(ap_boot_stack);
}

#define KINIT_THREAD_PRIORITY 3

static uint64_t kinit_thread(uint64_t);

static void start_kinit_thread(void) {
  struct thread_data *thrd = kmalloc(sizeof(*thrd));
  if (!thrd) {
    LOG_ERROR("failed to allocate memory");
    ASMV("hlt");
  }

  extern uint8_t bsp_boot_stack;
  memset(thrd, 0, sizeof(*thrd));
  thrd->stack = &bsp_boot_stack;
  thrd->stack_size = CONFIG_BSP_BOOT_STACK_SIZE;
  BIT_ARRAY_SET(thrd->affinity, get_bsp_cpu());
  thrd->priority = KINIT_THREAD_PRIORITY;
  thrd->fixed_priority = true;

  thread_id id;
  if (set_thread_context(thrd, kinit_thread, 0) ||
      attach_thread(thrd, &id) || resume_thread(id))
    LOG_ERROR("failed to start kinit_thread");

  ASMV("hlt");
}

void kmain(void) {
  init_vga();
  init_acpi();
  init_cpu_info();
  init_mem_mgr();
  init_interrupts();
  init_apic();
  ASMV("sti");
  init_scheduler();
  start_ap_cpus();
  start_kinit_thread();
}

ASM(".text\n.global kstart_ap\n"
    "kstart_ap: movw $" STR_EXPAND(SEGMENT_DATA) ", %ax\n"
    "movw %ax, %ds\nmovw %ax, %ss\n"
    "movq (ap_boot_stack), %rsp\n"
    "addq $" STR_EXPAND(CONFIG_AP_BOOT_STACK_SIZE) ", %rsp\n"
    "movq %cr4, %rax\n" // enable SSE
    "orl $" STR_EXPAND(CR4_OSFXSR) ", %eax\n"
    "movq %rax, %cr4\n"
    "call kmain_ap\n"
    "halt_ap: hlt\njmp halt_ap");

void kmain_ap(void) {
  init_interrupts();
  init_apic();
  ASMV("sti");
  init_scheduler();
  started_cpus++;
}

static uint64_t kinit_thread(UNUSED uint64_t input) {
  LOG_DEBUG("kinit_thread called with input %X", (uint32_t)input);
  return 0;
}
