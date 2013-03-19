#include "config.h"
#include "cpu_info.h"
#include "interrupt.h"
#include "mem_mgr.h"
#include "sys_table.h"

static uint8_t (*isr_stacks)[CONFIG_ISR_STACK_SIZE];
static struct sys_task_segment *task_segments;
static struct sys_idt_desc idt[INT_VECTORS];

static void add_gdt_tss(void) {
  int cpus = get_cpus();
  isr_stacks = kmalloc(cpus * CONFIG_ISR_STACK_SIZE);
  task_segments = kmalloc(cpus * sizeof(struct sys_task_segment));
  if (!isr_stacks || !task_segments) {
    LOG_ERROR("failed to allocate memory");
    return;
  }
  memset(task_segments, 0, cpus * sizeof(struct sys_task_segment));

  extern uint8_t gdt[];
  struct sys_gdt_desc2 *gdt2 =
    (struct sys_gdt_desc2*)(gdt + 3 * SYS_GDT_DESC_SIZE);

  for (int i = 0; i < cpus; i++) {
    task_segments[i].ists[0] = (uint64_t)&isr_stacks[i + 1];
    uint64_t tss_addr = (uint64_t)&task_segments[i];

    gdt2[i] = (struct sys_gdt_desc2) { {
        .type = SYS_SEGMENT_TSS, .nonsys = false, .present = true,
        .base0 = (uint16_t)tss_addr, .base1 = (uint8_t)(tss_addr >> 16),
        .limit0 = sizeof(struct sys_task_segment) - 1,
        .base2 = (uint8_t)(tss_addr >> 24)
      },
      .base3 = (uint32_t)(tss_addr >> 32)
    };
  }

  struct sys_table_info gdti = {
    (3 * SYS_GDT_DESC_SIZE) + (cpus * SYS_GDT_DESC2_SIZE) - 1, (uint64_t)gdt
  };
  ASMV("lgdt %0" : : "m"(gdti));
}

ISR_IMPL(default) {
  printf("#%s: ss: %X, rsp: %X, rflags: %X, cs: %X, rip: %X",
         (char*)(data << 8 >> 8), stack_frame->ss, (uint32_t)stack_frame->rsp,
         (uint32_t)stack_frame->rflags, stack_frame->cs,
         (uint32_t)stack_frame->rip);
  if (is_int_error(data >> 56))
    printf(", error_code: %X", stack_frame->error_code);
  printf("\n");
  ASMV("hlt");
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

#define CODE_SEGMENT 8

static void create_idt(void) {
  for (int i = 0; i < INT_VECTORS; i++)
    if (!is_int_reserved(i))
      idt[i] = (struct sys_idt_desc) {
        .cs = CODE_SEGMENT, .ist = 1, .type = SYS_GATE_INT, .present = false
      };

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
}

static void load_idt_tr(void) {
  struct sys_table_info idti = {
    SYS_IDT_DESC_SIZE * INT_VECTORS - 1, (uint64_t)idt
  };
  uint16_t sel = SYS_GDT_DESC_SIZE * 3 + SYS_GDT_DESC2_SIZE * get_cpu_index();
  ASMV("lidt %0\nmovw %1, %%ax\nltr %%ax" : : "m"(idti), "m"(sel));
}

void init_interrupts(void) {
  if (get_cpu_index() == get_bsp_cpu_index()) {
    add_gdt_tss();
    create_idt();
  }
  load_idt_tr();
  LOG_DEBUG("done");
}

void *get_isr(int vector) {
  struct sys_idt_desc *desc = idt + vector;
  return (void*)(desc->handler0 + ((uint64_t)desc->handler1 << 16) +
                 ((uint64_t)desc->handler2 << 32));
}

void set_isr(int vector, void *isr) {
  struct sys_idt_desc *desc = idt + vector;
  uint64_t isri = (uint64_t)isr;
  desc->present = isri != 0;
  desc->handler0 = (uint16_t)isri;
  desc->handler1 = (uint16_t)(isri >> 16);
  desc->handler2 = (uint32_t)(isri >> 32);
}
