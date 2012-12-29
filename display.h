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
  int8_t chr;
  uint8_t fcolor : 4;
  uint8_t bcolor : 4;
};

static inline volatile struct chr_cell *get_chr_cell (int row, int col) {
  return (struct chr_cell*)VIDEO_RAM_ADDR + row * COL_NUMBER + col;
}

int get_frame_top (void);
int get_frame_left (void);
int get_frame_height (void);
int get_frame_width (void);
int get_frame_fcolor (void);
int get_frame_bcolor (void);
void set_frame (int top, int left, int height, int width, int fcolor,
                int bcolor);
void clear_frame (void);

bool get_cursor (void);
void set_cursor (bool visible);

int get_caret_row (void);
int get_caret_col (void);
void set_caret (int row, int col);

int putchar (int chr);
int printf (char *format, ...);

#endif // DISPLAY_H
