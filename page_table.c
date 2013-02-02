#include "display.h"
#include "mem_map.h"
#include "page_table.h"
#include "util.h"

struct page_desc {
  uint32_t present : 1;
  uint32_t write : 1;
  uint32_t nonsys : 1;
  uint32_t pwt : 1;
  uint32_t pcd : 1;
  uint32_t accessed : 1;
  uint32_t dirty : 1;
  uint32_t ps_pat : 1;
  uint32_t global : 1;
  uint32_t avail0 : 3;
  uint32_t address0 : 20;
  uint32_t address1 : 20;
  uint32_t avail1 : 11;
  uint32_t noexec : 1;
};

#define PAGE_TABLE_SIZE 512

void init_page_tables(void) {
  struct page_desc *pml4 = (struct page_desc*)PML4_ADDR;
  pml4[0] = (struct page_desc) {
    .present = true, .write = true, .address0 = (uint32_t)(PDPT0_ADDR >> 12),
    .address1 = (uint32_t)((uint64_t)PDPT0_ADDR >> 32)
  };
  memset(&pml4[1], 0, 8 * (PAGE_TABLE_SIZE - 1));

  struct page_desc *pdpt0 = (struct page_desc*)PDPT0_ADDR; 
  pdpt0[0] = (struct page_desc) {
    .present = true, .write = true, .address0 = (uint32_t)(PD0_ADDR >> 12),
    .address1 = (uint32_t)((uint64_t)PD0_ADDR >> 32)
  };
  memset(&pdpt0[1], 0, 8 * (PAGE_TABLE_SIZE - 1));

  struct page_desc *pd0 = (struct page_desc*)PD0_ADDR; 
  pd0[0] = (struct page_desc) {
    .present = true, .write = true, .address0 = (uint32_t)(PT0_ADDR >> 12),
    .address1 = (uint32_t)((uint64_t)PT0_ADDR >> 32)
  };
  memset(&pd0[1], 0, 8 * (PAGE_TABLE_SIZE - 1));

  struct page_desc *pt0 = (struct page_desc*)PT0_ADDR; 
  for (uint32_t i = 0; i < PAGE_TABLE_SIZE; i++)
    pt0[i] = (struct page_desc) {
      .present = true, .write = true, .address0 = i
    };
  
  __asm__("mov %0, %%cr3" : : "d"(pml4));
}
