#include "page_map.h"

#define FLAGS_MASK 0x7A
#define NOEXEC_FLAG_BS 7

static volatile struct sys_page_desc *get_page_desc(uint64_t virt_addr) {
  uint64_t entry = virt_addr / PAGE_SIZE;
  uint64_t table = entry / SYS_PAGE_TABLE_DESCS;
  table += 2 + (table / SYS_PAGE_TABLE_DESCS);
  entry %= SYS_PAGE_TABLE_DESCS;

  extern int page_map;
  return (struct sys_page_desc*)((uint64_t)&page_map +
                                 (table * SYS_PAGE_TABLE_SIZE) +
                                 (entry * SYS_PAGE_DESC_SIZE));
}

bool get_page_mapping(uint64_t virt_addr, uint64_t *phys_addr, int *flags,
                      int *avail_data) {
  volatile struct sys_page_desc *pdesc = get_page_desc(virt_addr);
  if (!pdesc->present)
    return false;

  if (phys_addr)
    *phys_addr = ((uint64_t)pdesc->address1 << SYS_PAGE_DESC_ADDR1_BS) +
      (pdesc->address0 << SYS_PAGE_DESC_ADDR0_BS);
  if (flags)
    *flags = (*(uint8_t*)pdesc & FLAGS_MASK) |
      (pdesc->noexec << NOEXEC_FLAG_BS);
  if (avail_data)
    *avail_data = pdesc->avail0 | (pdesc->avail1 << SYS_PAGE_DESC_AVAIL1_BS);
  return true;
}

void map_page(uint64_t virt_addr, uint64_t phys_addr, int flags,
              int avail_data) {
  struct sys_page_desc pdesc = {
    .present = true, .ps_pat = true, .avail0 = avail_data,
    .address0 = (uint32_t)(phys_addr >> SYS_PAGE_DESC_ADDR0_BS) &
      ~(SYS_PAGE_TABLE_DESCS - 1),
    .address1 = (uint32_t)(phys_addr >> SYS_PAGE_DESC_ADDR1_BS),
    .avail1 = avail_data >> SYS_PAGE_DESC_AVAIL1_BS
  };
  *(uint8_t*)&pdesc |= (flags & FLAGS_MASK);
  pdesc.noexec = flags >> NOEXEC_FLAG_BS;

  *get_page_desc(virt_addr) = pdesc;
  ASMV("invlpg (%0)" : : "r"(virt_addr));
}

void unmap_page(uint64_t virt_addr) {
  *get_page_desc(virt_addr) = (struct sys_page_desc) { .present = false };
  ASMV("invlpg (%0)" : : "r"(virt_addr));
}
