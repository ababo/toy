#include "page_map.h"
#include "sync.h"
#include "vga.h"

#define VIDEO_ADDR 0xB8000

struct chr_cell {
  int8_t chr;
  uint8_t fcolor : 4;
  uint8_t bcolor : 4;
};

static int frame_top, frame_left;
static int frame_height, frame_width;
static int frame_fcolor, frame_bcolor;
static int caret_row, caret_col;
static bool cursor;

static struct spinlock lock;

static inline volatile struct chr_cell *get_chr_cell(int row, int col) {
  return (struct chr_cell*)VIDEO_ADDR + row * VGA_COLS_NUMBER + col;
}

void get_vga_cell(int row, int col, char *chr, int *fore_color,
                  int *back_color) {
  acquire_spinlock(&lock);
  volatile struct chr_cell *cell = get_chr_cell(row, col);
  if (chr)
    *chr = cell->chr;
  if (fore_color)
    *fore_color = cell->fcolor;
  if (back_color)
    *back_color = cell->bcolor;
  release_spinlock(&lock);
}

void set_vga_cell(int row, int col, char chr, int fore_color, int back_color) {
  acquire_spinlock(&lock);
  *get_chr_cell(row, col) = (struct chr_cell) {
    chr, (uint8_t)fore_color, (uint8_t)back_color
  };
  release_spinlock(&lock);
}

int get_vga_frame_top(void) {
  return frame_top;
}

int get_vga_frame_left(void) {
  return frame_left;
}

int get_vga_frame_height(void) {
  return frame_height;
}

int get_vga_frame_width(void) {
  return frame_width;
}

int get_vga_frame_fcolor(void) {
  return frame_fcolor;
}

int get_vga_frame_bcolor(void) {
  return frame_bcolor;
}

void set_vga_frame(int top, int left, int height, int width, int fcolor,
                   int bcolor) {
  acquire_spinlock(&lock);
  frame_top = top, frame_left = left, frame_height = height,
    frame_width = width, frame_fcolor = fcolor, frame_bcolor = bcolor;
  release_spinlock(&lock);
}

void clear_vga_frame(void) {
  acquire_spinlock(&lock);
  for (int row = 0; row < frame_height; row++)
    for (int col = 0; col < frame_width; col++)
      *get_chr_cell(frame_top + row, frame_left + col) =
        (struct chr_cell) { 0, frame_fcolor, frame_bcolor };
  release_spinlock(&lock);
}

bool get_vga_cursor(void) {
  return cursor;
}

static void put_cursor(int row, int col) {
  int off = row * VGA_COLS_NUMBER + col;
  outb(0x3D4, 0x0F);
  outb(0x3D5, (unsigned char)(off & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, (unsigned char)((off >> 8) & 0xFF));
}

void set_vga_cursor(bool visible) {
  acquire_spinlock(&lock);
  cursor = visible;
  if (visible) {
    outw(0x3D4,0xE0A);
    outw(0x3D4,0xF0B);
    put_cursor(frame_top + caret_row, frame_left + caret_col);
  }
  else {
    outw(0x3D4,0x200A);
    outw(0x3D4,0xB);
    put_cursor(VGA_ROWS_NUMBER, 0);
  }
  release_spinlock(&lock);
}

int get_vga_caret_row(void) {
  return caret_row;
}

int get_vga_caret_col(void) {
  return caret_col;
}

void set_vga_caret(int row, int col) {
  acquire_spinlock(&lock);
  caret_row = row, caret_col = col;
  if (cursor)
    put_cursor(frame_top + row, frame_left + col);
  release_spinlock(&lock);
}

void init_vga(void) {
  map_page(VIDEO_ADDR, VIDEO_ADDR, PAGE_MAPPING_WRITE, 0);
  set_vga_frame(0, 0, VGA_ROWS_NUMBER, VGA_COLS_NUMBER, VGA_COLOR_WHITE,
                VGA_COLOR_LOW_BLUE);
  create_spinlock(&lock);
  clear_vga_frame();
  set_vga_cursor(true);
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
    if (++caret_col == frame_width)
      goto new_line;
    break;
  }
}

int kputchar(int chr) {
  acquire_spinlock(&lock);
  put_char(chr);
  if (cursor)
    put_cursor(frame_top + caret_row, frame_left + caret_col);
  release_spinlock(&lock);
  return chr;
}

int kprintf(const char *format, ...) {
  acquire_spinlock(&lock);

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
      case 'c':
        buf[0] = (char)va_arg(vargs, int), buf[1] = 0, str = buf;
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

  release_spinlock(&lock);
  return num;
}
