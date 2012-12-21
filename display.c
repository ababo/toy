#include "display.h"

static int caret_row = 0, caret_col = 0;
static bool cursor = false;

void init_display (int fcolor, int bcolor) {
  for (int row = 0; row < ROW_NUMBER; row++)
    for (int col = 0; col < COL_NUMBER; col++)
      *get_chr_cell(row, col) = (struct chr_cell) { 0, fcolor, bcolor };
}

int get_caret_row () {
  return caret_row;
}

int get_caret_col () {
  return caret_col;
}

static inline void set_cursor_pos (int row, int col) {
  int pos = row * COL_NUMBER + col;
  outb(0x3D4, 0x0F);
  outb(0x3D5, (unsigned char)(pos & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

void set_caret (int row, int col) {
  if (row < 0 || row >= ROW_NUMBER || col < 0 || col >= COL_NUMBER)
    return;
  caret_row = row, caret_col = col;
  if (cursor)
    set_cursor_pos(row, col);
}

bool get_cursor () {
  return cursor;
}

void set_cursor (bool visible) {
  cursor = visible;
  set_cursor_pos(visible ? caret_row : ROW_NUMBER, visible ? caret_col : 0);
}

int putchar (int chr) {

}

int printf (char *format, ...) {

}


/*
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
*/
