#include "multiboot.h"
#include "util.h"

#define HEADER_FLAGS (MULTIBOOT_HEADER_MEMOTY_MAP)

USED static const struct multiboot_header header = {
  .magic = MULTIBOOT_HEADER_MAGIC, .flags = HEADER_FLAGS,
  .checksum = -(MULTIBOOT_HEADER_MAGIC + HEADER_FLAGS)
};

ASM(".global kstart32\n"
    "kstart32: call kmain32\n"
    "hlt");

void kmain32(void) {
  *(char*)0xB8000 = 49;
}
