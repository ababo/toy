#include "display.h"

#define VIDEO_RAM_PTR 0xB8000

static int fcolor = COLOR_WHITE;
static int bcolor = COLOR_BLACK;
static int drow = 0, dcolumn = 0;

void set_foreground (unsigned int color) {
  if (color > COLOR_WHITE)
    return;
  fcolor = color;
}

void set_background (unsigned int color) {
  if (color > COLOR_WHITE)
    return;
  bcolor = color;
}

void set_caret (unsigned int row, unsigned int column) {
  if (row >= ROWS_NUMBER || column >= COLUMNS_NUMBER)
    return;
  drow = row, dcolumn = column;
}

struct pixel {
  char chr;
  unsigned char fcolor : 4;
  unsigned char bcolor : 4;
};

static inline struct pixel *get_pixel (unsigned int row, unsigned int column) {
  return (struct pixel*)VIDEO_RAM_PTR + row * COLUMNS_NUMBER + column;
}

void clrscr () {
  drow = dcolumn = 0;
  for (unsigned int row = 0; row < ROWS_NUMBER; row++)
    for (unsigned int column = 0; column < COLUMNS_NUMBER; column++) {
      volatile struct pixel *pixel = get_pixel(row, column);
      pixel->chr = 0, pixel->fcolor = fcolor, pixel->bcolor = bcolor;
    }
}

void putc (char chr) {
  if (chr == '\n') {
  new_line:
    drow++, dcolumn = 0;
    if (drow == ROWS_NUMBER)
      drow = 0;
    return;
  }

  volatile struct pixel *pixel = get_pixel(drow, dcolumn);
  pixel->chr = chr, pixel->fcolor = fcolor, pixel->bcolor = bcolor;

  if (++dcolumn == COLUMNS_NUMBER)
    goto new_line;
}
