#include "mem_map.h"
#include "page_map.h"
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

static inline struct page_desc get_page_desc(bool present, uint64_t address) {
  struct page_desc pd = { };
  if (present) {
    pd.present = pd.write = true;
    pd.address0 = (uint32_t)(address >> 12);
    pd.address1 = (uint32_t)(address >> 32);
  }
  return pd;
}

// the map must reside either in mapped interval or at first 2MiB
// always maps both the first 2MiB as 1:1 and the map itself
// doesn't map everything else if mapped_phys_addr == 0
static void create_map(uint64_t map_virt_addr, uint64_t map_phys_addr,
                       uint64_t mapped_virt_addr, uint64_t mapped_phys_addr,
                       uint64_t mapped_virt_size) {
  uint64_t msize = get_page_map_size(mapped_virt_size), offset = 0;
  memset((void*)map_virt_addr, 0, msize);

  struct page_desc *pml4e = (struct page_desc*)map_virt_addr;
  struct page_desc *nextt = pml4e + PAGE_TABLE_SIZE, *pdpte, *pde;

  for (int pml4i = 0; pml4i < PAGE_TABLE_SIZE; pml4i++)
    if (!pml4i || pml4i >= INT_BITS(map_virt_addr, 39, 47)) {
      pdpte = nextt, nextt += PAGE_TABLE_SIZE;
      pml4e[pml4i] = get_page_desc(true, (uint64_t)pdpte);

      for (int pdpti = 0; pdpti < PAGE_TABLE_SIZE; pdpti++)
        if (pml4i || !pdpti || pdpti >= INT_BITS(map_virt_addr, 30, 38)) {
          pde = nextt, nextt += PAGE_TABLE_SIZE;
          pdpte[pdpti] = get_page_desc(true, (uint64_t)pde);

          for (int pdi = 0; pdi < PAGE_TABLE_SIZE; pdi++)
            if (!pml4i && !pdpti && !pdi && mapped_virt_addr)
              // first 2MiB are external for the mapped interval
              pde[pdi] = get_page_desc(true, 0);
            else if (!pml4i || !pdpti || !pdi ||
                     pdi >= INT_BITS(map_virt_addr, 21, 29)) {
              uint64_t vaddr = mapped_virt_addr + offset, paddr = 0;
              if (mapped_phys_addr)
                paddr = mapped_phys_addr + offset;
              else if (vaddr >= map_virt_addr && vaddr < map_virt_addr + msize)
                paddr = map_phys_addr + vaddr - map_virt_addr;
              pde[pdi] = get_page_desc(paddr, paddr);

              if((offset += PAGE_SIZE) >= mapped_virt_size)
                break;
            }
        }
    }
}

void init_page_map(uint64_t map_phys_addr, uint64_t mapped_virt_size) {
  create_map(PAGE_MAP_TEMP_ADDR, PAGE_MAP_TEMP_ADDR, PAGE_MAP_ADDR,
             map_phys_addr, get_page_map_size(mapped_virt_size));
  create_map(PAGE_MAP_ADDR, map_phys_addr, 0, 0, mapped_virt_size);
}

bool get_page_mapping(uint64_t virt_addr, uint64_t *phys_addr, int *flags,
                      uint16_t *avail_data) {

  return false;
}

void map_page(uint64_t virt_addr, uint64_t phys_addr, int flags,
              uint16_t avail_data) {

}

void unmap_page(uint64_t virt_addr) {

}
