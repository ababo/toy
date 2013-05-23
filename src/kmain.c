#include "common.h"

void kmain(void) {
  *(char*)0xB8000 = '!';
  kprintf("Hello World!");
}
