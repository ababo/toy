#ifndef DISPLAY_H
#define DISPLAY_H

#include "util.h"

#define COLOR_BLACK 0
#define COLOR_LOW_BLUE 1
#define COLOR_LOW_GREEN 2
#define COLOR_LOW_CYAN 3
#define COLOR_LOW_RED 4
#define COLOR_LOW_MAGENTA 5
#define COLOR_BROWN 6
#define COLOR_LIGHT_GRAY 7
#define COLOR_DARK_GRAY 8
#define COLOR_HIGH_BLUE 9
#define COLOR_HIGH_GREEN 10
#define COLOR_HIGH_CYAN 11
#define COLOR_HIGH_RED 12
#define COLOR_HIGH_MAGENTA 13
#define COLOR_HIGH_YELLOW 14
#define COLOR_WHITE 15

#define ROW_NUMBER 25
#define COL_NUMBER 80

#define VIDEO_RAM_ADDR 0xB8000

struct chr_cell {
  char chr;
  unsigned char fcolor : 4;
  unsigned char bcolor : 4;
};

static inline volatile struct chr_cell *get_chr_cell (int row, int col) {
  return (struct chr_cell*)VIDEO_RAM_ADDR + row * COL_NUMBER + col;
}

void init_display (int fcolor, int bcolor);

int get_caret_row ();
int get_caret_col ();
void set_caret (int row, int col);

bool get_cursor ();
void set_cursor (bool visible);

int putchar (int chr);
int printf (char *format, ...);

#endif // DISPLAY_H
