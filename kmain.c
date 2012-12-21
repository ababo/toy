#include "display.h"

void kmain(void) {
  init_display(COLOR_WHITE, COLOR_LOW_BLUE);
  set_cursor(true);
  set_caret(12, 33);
  printf("Hello Kernel!");
};
