#ifndef PAGE_MAP_H
#define PAGE_MAP_H

#define PAGE_SIZE 0x200000

#define PAGE_MAPPING_WRITE (1 << 1)
#define PAGE_MAPPING_NONCACHED (0b11 << 3)
#define PAGE_MAPPING_ACCESSED (1 << 5)
#define PAGE_MAPPING_DIRTY (1 << 6)
#define PAGE_MAPPING_NOEXEC (1 << 15)

static inline uint64_t get_page_map_size(size_t virt_size) {
  int pd_num = BLOCK_NUM(virt_size, 512 * PAGE_SIZE);
  int pdpt_num = BLOCK_NUM(pd_num, 512);
  return 4096 * (pd_num + pdpt_num + 1);
}

void init_page_map(uint64_t phys_addr, uint64_t virt_size);

bool get_page_mapping(uint64_t virt_addr, uint64_t *phys_addr, int *flags,
                      uint16_t *avail_data);

void map_page(uint64_t virt_addr, uint64_t phys_addr, int flags,
              uint16_t avail_data);
void unmap_page(uint64_t virt_addr);

#endif // PAGE_MAP_H
