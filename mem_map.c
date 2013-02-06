#include "mem_map.h"

#define USABLE_MEMORY_TYPE 1
#define RESERVED_MEMORY_TYPE 2
#define ACPI_RECLAIM_MEMORY_TYPE 3
#define ACPI_NVS_MEMORY_TYPE 4
#define BAD_MEMORY_TYPE 5

struct memory_map {
  uint16_t size;
  struct {
    uint64_t base;
    uint64_t size;
    uint32_t type;
    uint32_t present : 1;
    uint32_t nonvolatile : 1;
  } entries[1];
};

uint64_t get_usable_memory_size(void) {
  struct memory_map *mm = (struct memory_map*)MEMORY_MAP_ADDR;
  uint64_t size = 0;
  for (int i = 0; i < mm->size; i++) // suppose the entries are non-overlapping
    if (mm->entries[i].type == USABLE_MEMORY_TYPE)
      size += mm->entries[i].size;
  return size;
}
