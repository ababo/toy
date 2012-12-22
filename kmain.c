#include "display.h"

void kmain(void) {
  set_frame(6, 20, 13, 40, COLOR_WHITE, COLOR_LOW_BLUE);
  clear_frame();
  set_cursor(true);
  set_caret(2, 3);
  printf("Hello Kernel!");
};
