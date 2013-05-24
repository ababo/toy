#ifndef X86_64_MULTIBOOT_H
#define X86_64_MULTIBOOT_H

#include "../common.h"

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

struct multiboot_info {
  uint32_t flags;
  uint32_t mem_lower;
  uint32_t mem_higher;
  uint8_t boot_drive;
  uint8_t boot_part[3];
  uint32_t cmdline;
  uint32_t mods_count;
  uint32_t mods_addr;
  uint32_t syms_tab_size;
  uint32_t syms_str_size;
  uint32_t syms_addr;
  uint32_t reserved;
  uint32_t mmap_len;
  uint32_t mmap_addr;
  uint32_t drives_len;
  uint32_t drives_addr;
  uint32_t config_table_addr;
  uint32_t bootloader_name;
  uint32_t apm_table_addr;
  uint32_t vbe_control_info_addr;
  uint32_t vbe_mode_info_addr;
  uint16_t vbe_mode;
  uint16_t vbe_interface_seg;
  uint16_t vbe_interface_off;
  uint16_t vbe_interface_len;
};

#endif // X86_64_MULTIBOOT_H
