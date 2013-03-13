#ifndef PAGE_MAP_H
#define PAGE_MAP_H

#include "config.h"
#include "sys_table.h"
#include "util.h"

#define PAGE_SIZE 0x200000

#define PAGE_MAP_PDS \
  SIZE_ELEMENTS(CONFIG_ADDR_SPACE_SIZE, SYS_PAGE_TABLE_DESCS * PAGE_SIZE)
#define PAGE_MAP_PDPTS \
  SIZE_ELEMENTS(PAGE_MAP_PDS, SYS_PAGE_TABLE_DESCS)
#define PAGE_MAP_SIZE \
  ((PAGE_MAP_PDS + PAGE_MAP_PDPTS + 1) * SYS_PAGE_TABLE_SIZE)

#define PAGE_MAPPING_WRITE (1 << 1)
#define PAGE_MAPPING_PWT (1 << 3)
#define PAGE_MAPPING_PCD (1 << 4)
#define PAGE_MAPPING_ACCESSED (1 << 5)
#define PAGE_MAPPING_DIRTY (1 << 6)
#define PAGE_MAPPING_NOEXEC (1 << 7)

bool get_page_mapping(uint64_t virt_addr, uint64_t *phys_addr, int *flags,
                      int *avail_data);
void map_page(uint64_t virt_addr, uint64_t phys_addr, int flags,
              int avail_data); // only 14-bits of avail_data are usable
void unmap_page(uint64_t virt_addr);

#endif // PAGE_MAP_H
