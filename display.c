#include "display.h"

#define VIDEO_RAM_PTR 0xB8000

static int fcolor = COLOR_WHITE;
static int bcolor = COLOR_BLACK;
static int drow, dcolumn = 0;

void set_foreground (int color) {
  if (color < COLOR_BLACK || color > COLOR_WHITE)
    return;
  fcolor = color;
}

void set_background (int color) {
  if (color < COLOR_BLACK || color > COLOR_WHITE)
    return;
  bcolor = color;
}

void set_caret (int row, int column) {
  if (row < 0 || row >= ROWS_NUMBER || column < 0 || column >= COLUMNS_NUMBER)
    return;
  drow = row, dcolumn = column;
}

struct pixel {
  char chr;
  unsigned char fcolor : 4;
  unsigned char bcolor : 4;
};

void putc (char chr) {
  if (chr == '\n') {
  new_line:
    drow++, dcolumn = 0;
    if (drow == ROWS_NUMBER)
      drow = 0;
    return;
  }

  volatile struct pixel *pixel =
    (struct pixel*)VIDEO_RAM_PTR + (dcolumn + drow * COLUMNS_NUMBER) * 2;
  pixel->chr = chr, pixel->fcolor = fcolor, pixel->bcolor = bcolor;

  if (++dcolumn == COLUMNS_NUMBER)
    goto new_line;
}
