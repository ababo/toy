#include "display.h"

static int d_row = 0, d_col = 0;

void set_caret (int row, int col) {
  d_row = row, d_col = col;
}

void putc (char chr) {
  
}
