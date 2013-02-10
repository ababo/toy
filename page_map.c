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

void init_page_map(uint64_t phys_addr, uint64_t virt_size) {

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
