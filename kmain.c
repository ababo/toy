#include "display.h"

void kmain(void) {
  set_background(COLOR_LOW_BLUE);
  set_foreground(COLOR_WHITE);
  clrscr();
  set_caret(12, 33);
  putc('H');
  putc('e');
  putc('l');
  putc('l');
  putc('o');
  putc(' ');
  putc('K');
  putc('e');
  putc('r');
  putc('n');
  putc('e');
  putc('l');
  putc('!');
};
