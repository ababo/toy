#ifndef PAGE_MAP_H
#define PAGE_MAP_H

#include "util.h"

#define PAGE_SIZE 0x200000

#define PAGE_MAPPING_WRITE (1 << 1)
#define PAGE_MAPPING_PWT (1 << 3)
#define PAGE_MAPPING_PCD (1 << 4)
#define PAGE_MAPPING_ACCESSED (1 << 5)
#define PAGE_MAPPING_DIRTY (1 << 6)
#define PAGE_MAPPING_NOEXEC (1 << 7)

static inline uint64_t get_page_map_size(size_t mapped_virt_size) {
  int pd_num = BLOCK_NUM(mapped_virt_size, 512 * PAGE_SIZE);
  int pdpt_num = BLOCK_NUM(pd_num, 512);
  return (pd_num + pdpt_num + 1) * 4096;
}

uint64_t get_page_map_phys_addr(void);
uint64_t get_mapped_virt_size(void);
bool get_page_mapping(uint64_t virt_addr, uint64_t *phys_addr, int *flags,
                      int *avail_data);

void map_page(uint64_t virt_addr, uint64_t phys_addr, int flags,
              int avail_data); // only 14-bits of avail_data are usable
void unmap_page(uint64_t virt_addr);

void init_page_map(uint64_t map_phys_addr, uint64_t mapped_virt_size);

#endif // PAGE_MAP_H
