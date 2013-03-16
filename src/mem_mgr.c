#include "config.h"
#include "mem_mgr.h"
#include "multiboot.h"
#include "page_map.h"

/// TODO: Replace this dummy code with a real implementaion

static uint8_t *next_free;

void *kmalloc(size_t size) {
  void *ptr = next_free;
  next_free += size;
  return ptr;
}

static void map_ram(void) {
  extern uint32_t multiboot_info;
  uint64_t high_mem_hole =
    ((struct multiboot_info*)(uint64_t)multiboot_info)->mem_higher;
  high_mem_hole = (high_mem_hole + 1024) * 1024 / PAGE_SIZE * PAGE_SIZE;

  for (uint64_t addr = 0; addr < high_mem_hole; addr += PAGE_SIZE)
    map_page(addr, addr, PAGE_MAPPING_WRITE, 0);

  LOG_DEBUG("mapped as 1:1 up to %X", (uint32_t)high_mem_hole);
}

void init_mem_mgr(void) {
  extern int lds_kernel_size;
  next_free = (uint8_t*)(CONFIG_KERNEL_ADDR + &lds_kernel_size);
  map_ram();
  LOG_DEBUG("done");
}
