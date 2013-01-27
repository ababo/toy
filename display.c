#include "display.h"

static int frame_top = 0, frame_left = 0;
static int frame_height = ROW_NUMBER, frame_width = COL_NUMBER;
static int frame_fcolor = COLOR_WHITE, frame_bcolor = COLOR_BLACK;
static int caret_row = 0, caret_col = 0;
static bool cursor = false;

int get_frame_top(void) {
  return frame_top;
}

int get_frame_left(void) {
  return frame_left;
}

int get_frame_height(void) {
  return frame_height;
}

int get_frame_width(void) {
  return frame_width;
}

int get_frame_fcolor(void) {
  return frame_fcolor;
}

int get_frame_bcolor(void) {
  return frame_bcolor;
}

void set_frame(int top, int left, int height, int width, int fcolor,
               int bcolor) {
  frame_top = top, frame_left = left, frame_height = height,
    frame_width = width, frame_fcolor = fcolor, frame_bcolor = bcolor;
}

void clear_frame(void) {
  for (int row = 0; row < frame_height; row++)
    for (int col = 0; col < frame_width; col++)
      *get_chr_cell(frame_top + row, frame_left + col) =
        (struct chr_cell) { 0, frame_fcolor, frame_bcolor };
}

bool get_cursor(void) {
  return cursor;
}

static void put_cursor(int row, int col) {
  int off = row * COL_NUMBER + col;
  outb(0x3D4, 0x0F);
  outb(0x3D5, (unsigned char)(off & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, (unsigned char)((off >> 8) & 0xFF));
}

void set_cursor(bool visible) {
  cursor = visible;
  if (visible)
    put_cursor(frame_top + caret_row, frame_left + caret_col);
  else
    put_cursor(ROW_NUMBER, 0);
}

int get_caret_row(void) {
  return caret_row;
}

int get_caret_col(void) {
  return caret_col;
}

void set_caret(int row, int col) {
  caret_row = row, caret_col = col;
  if (cursor)
    put_cursor(frame_top + row, frame_left + col);
}

static void scroll_frame(void) {
  for (int row = 1; row < frame_height; row++)
    for (int col = 0; col < frame_width; col++)
      *get_chr_cell(frame_top + row - 1, frame_left + col) =
        *get_chr_cell(frame_top + row, frame_left + col);

  for (int col = 0; col < frame_width; col++)
    *get_chr_cell(frame_top + frame_height - 1, frame_left + col) =
      (struct chr_cell) { 0, frame_fcolor, frame_bcolor };
}

static void put_char(char chr) {
  switch (chr) {
    case '\r': 
      caret_col = 0;
      break;
    case '\n':
  new_line:
      caret_col = 0, caret_row++;
      if (caret_row == frame_height) {
        scroll_frame();
        caret_row--;
      }
      break;
    // TODO: implement other escapes
    default:
      *get_chr_cell(frame_top + caret_row, frame_left + caret_col) =
        (struct chr_cell) { chr, frame_fcolor, frame_bcolor };
      if (++caret_col == frame_width) {
        caret_col = 0;
        goto new_line;
      }
      break;
  }
}

int putchar(int chr) {
  put_char(chr);
  if (cursor)
    put_cursor(frame_top + caret_row, frame_left + caret_col);
  return chr;
}

int printf(char *format, ...) {
  va_list vargs;
  va_start(vargs, format);

  int num = 0, int_arg;
  char chr, *str, buf[20];
  while ((chr = *format++))
    if (chr == '%')
      switch ((chr = *format++)) {
        case '%':
          put_char('%');
          num++;
          break;
        case 's':
          str = va_arg(vargs, char*);
      puts:
          while (*str) {
            put_char(*str++);
            num++;
          }
          break;
        case 'd':
          int_arg = va_arg(vargs, int);
          if (int_arg < 0) {
            put_char('-');
            int_arg = -int_arg;
            num++;
          }
          str = ultoa(int_arg, buf, 10);
          goto puts;
        case 'x':
        case 'X':
          int_arg = va_arg(vargs, int);
          str = ultoa((unsigned int)int_arg, buf, chr == 'x' ? 16 : -16);
          goto puts;
          // TODO: implement other specifiers
        default:
          return -1;
      }
    else {
      put_char(chr);
      num++;
    }
  va_end(vargs);

  if (cursor)
    put_cursor(frame_top + caret_row, frame_left + caret_col);

  return num;
}
