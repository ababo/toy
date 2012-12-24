#include "display.h"

void kmain(void) {
  set_frame(0, 0, ROW_NUMBER, COL_NUMBER, COLOR_WHITE, COLOR_LOW_BLUE);
  clear_frame();
  set_cursor(true);
  printf("Hello from %s v0.00%d!", "kernel", 1);
};
