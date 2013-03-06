#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include "util.h"

#define MULTIBOOT_HEADER_MAGIC 0x1BADB002

#define MULTIBOOT_HEADER_MODULE_ALIGN (1 << 0)
#define MULTIBOOT_HEADER_MEMOTY_MAP (1 << 1)
#define MULTIBOOT_HEADER_VIDEO_INFO (1 << 2)
#define MULTIBOOT_HEADER_USE_ADDRS (1 << 16)

struct multiboot_header {
  uint32_t magic;
  uint32_t flags;
  uint32_t checksum;
  uint32_t header_addr;
  uint32_t load_addr;
  uint32_t load_end_addr;
  uint32_t bss_end_addr;
  uint32_t entry_addr;
  uint32_t video_mode;
  uint32_t video_width;
  uint32_t video_height;
  uint32_t video_depth;
};

#endif // MULTIBOOT_H
