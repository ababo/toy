#include "addr_map.h"
#include "page_map.h"

#define FLAGS_MASK 0b1111010
#define NOEXEC_FLAG_SHIFT 7

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

static uint64_t phys_addr;
static uint64_t virt_size;

uint64_t get_page_map_phys_addr(void) {
  return phys_addr;
}

uint64_t get_mapped_virt_size(void) {
  return virt_size;
}

static inline struct page_desc *get_page_desc(uint64_t virt_addr) {
  uint64_t entry = virt_addr >> 21, table = entry >> 9;
  entry = entry & (512 - 1), table += 2 + (table >> 9);
  return (struct page_desc*)(ADDR_PAGE_MAP + (table << 12) + (entry << 3));
}

bool get_page_mapping(uint64_t virt_addr, uint64_t *phys_addr, int *flags,
                      int *avail_data) {
  struct page_desc *pd = get_page_desc(virt_addr);
  if (!pd->present)
    return false;

  if (phys_addr)
    *phys_addr = ((uint64_t)pd->address1 << 32) + (pd->address0 << 12);
  if (flags)
    *flags = (*(uint8_t*)pd & FLAGS_MASK) | (pd->noexec << NOEXEC_FLAG_SHIFT);
  if (avail_data)
    *avail_data = pd->avail0 | (pd->avail1 << 3);
  return true;
}

void map_page(uint64_t virt_addr, uint64_t phys_addr, int flags,
              int avail_data) {
  struct page_desc *pd = get_page_desc(virt_addr);

  *pd = (struct page_desc) {
    .present = true, .ps_pat = true, .avail0 = avail_data,
    .address0 = (uint32_t)(phys_addr >> 12) & ~(512 - 1),
    .address1 = (uint32_t)(phys_addr >> 32), .avail1 = avail_data >> 3
  };
  *(uint8_t*)pd |= (flags & FLAGS_MASK);
  pd->noexec = flags >> NOEXEC_FLAG_SHIFT;

  asm("invlpg (%0)" : : "r"(virt_addr));
}

void unmap_page(uint64_t virt_addr) {
  *get_page_desc(virt_addr) = (struct page_desc) { };
  asm("invlpg (%0)" : : "r"(virt_addr));
}

static inline void set_page_desc(struct page_desc *table, int entry,
                                 bool present, bool page, uint64_t address) {
  table[entry] = (struct page_desc) { };
  if (present) {
    table[entry].present = true;
    table[entry].write = true;
    table[entry].ps_pat = !!page;
    table[entry].address0 = (uint32_t)(address >> 12);
    table[entry].address1 = (uint32_t)(address >> 32);
  }
}

#define PHYS_ADDR(virt_ptr) \
  ((uint64_t)virt_ptr - map_virt_addr + map_phys_addr)

// the map must reside either inside the mapped interval or at the first 2MiB
// always maps both the first 2MiB as 1:1 and the map itself
// doesn't map anything else if mapped_phys_addr == 0
static void create_map(uint64_t map_virt_addr, uint64_t map_phys_addr,
                       uint64_t mapped_virt_addr, uint64_t mapped_phys_addr,
                       uint64_t mapped_virt_size) {
  uint64_t msize = get_page_map_size(mapped_virt_size), offset = 0;
  memset((void*)map_virt_addr, 0, msize);

  struct page_desc *pml4e = (struct page_desc*)map_virt_addr;
  struct page_desc *pdpte = pml4e + 512;
  struct page_desc *pde = pdpte + 512;
  struct page_desc *nextt = pde + 512;
  set_page_desc(pml4e, 0, true, false, PHYS_ADDR(pdpte));
  set_page_desc(pdpte, 0, true, false, PHYS_ADDR(pde));
  set_page_desc(pde, 0, true, true, 0);

  int pml4i = INT_BITS(mapped_virt_addr, 39, 47);
  int pdpti = INT_BITS(mapped_virt_addr, 30, 38);
  int pdi = INT_BITS(mapped_virt_addr, 21, 29);

  for (; pml4i < 512; pml4i++) {
    if (pml4i) {
      pdpte = nextt, pdpti = 0, nextt += 512;
      set_page_desc(pml4e, pml4i, true, false, PHYS_ADDR(pdpte));
    }

    for (; pdpti < 512; pdpti++) {
      if (pdpti || pml4i) {
        pde = nextt, pdi = 0, nextt += 512;
        set_page_desc(pdpte, pdpti, true, false, PHYS_ADDR(pde));
      }

      for (; pdi < 512; pdi++) {
        if (pml4i || pdpti || pdi) {
          uint64_t vaddr = mapped_virt_addr + offset, paddr = 0;
          if (mapped_phys_addr)
            paddr = mapped_phys_addr + offset;
          else if (vaddr >= map_virt_addr && vaddr < map_virt_addr + msize)
            paddr = map_phys_addr + vaddr - map_virt_addr;
          set_page_desc(pde, pdi, (bool)paddr, true, paddr);
        }

        if((offset += PAGE_SIZE) >= mapped_virt_size)
          goto finish;
      }
    }
  }

 finish:
  asm("movq %0, %%cr3" : : "d"(map_phys_addr));
}

void init_page_map(uint64_t map_phys_addr, uint64_t mapped_virt_size) {
  create_map(ADDR_PAGE_MAP_TEMP, ADDR_PAGE_MAP_TEMP, ADDR_PAGE_MAP,
             map_phys_addr, get_page_map_size(mapped_virt_size));
  create_map(ADDR_PAGE_MAP, map_phys_addr, 0, 0, mapped_virt_size);
  phys_addr = map_phys_addr, virt_size = mapped_virt_size;
  LOG_DEBUG("init_page_map: done");
}
