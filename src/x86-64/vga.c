#include "cpu.h"
#include "spinlock.h"
#include "vga.h"

#define VIDEO_ADDR 0xB8000

struct vga_cell {
  int8_t chr;
  uint8_t fcolor : 4;
  uint8_t bcolor : 4;
};

static inline volatile struct vga_cell *get_cell(int row, int col) {
  return (struct vga_cell*)VIDEO_ADDR + row * VGA_COLS + col;
}

void get_vga_cell(int row, int col, char *chr, int *fcolor, int *bcolor) {
  volatile struct vga_cell *cell = get_cell(row, col);
  if (chr)
    *chr = cell->chr;
  if (fcolor)
    *fcolor = cell->fcolor;
  if (bcolor)
    *bcolor = cell->bcolor;
}

void set_vga_cell(int row, int col, char chr, int fcolor, int bcolor) {
  *get_cell(row, col) = (struct vga_cell) {
    chr, (uint8_t)fcolor, (uint8_t)bcolor
  };
}

static int cursor_row, cursor_col;

void get_vga_cursor(int *row, int *col) {
  if (row)
    *row = cursor_row;
  if (col)
    *col = cursor_col;
}

void set_vga_cursor(int row, int col) {
  cursor_row = row, cursor_col = col;
  int off = row * VGA_COLS + col;
  outb(0x3D4, 0x0F);
  outb(0x3D5, (unsigned char)(off & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, (unsigned char)((off >> 8) & 0xFF));
}

static struct spinlock lock;

struct spinlock *get_vga_lock(void) {
  return &lock;
}

void init_vga(void) {
  create_spinlock(&lock);
  clear_display();
  outw(0x3D4, 0xE0A); // make cursor visible
  outw(0x3D4, 0xF0B);
  set_vga_cursor(0, 0);
}

void get_display_size(int *rows, int *cols) {
  if (rows)
    *rows = VGA_ROWS;
  if (cols)
    *cols = VGA_COLS;
}

struct spinlock *get_display_lock(void) {
  return &lock;
}

void set_display_char(int row, int col, char chr) {
  get_cell(row, col)->chr = chr;
}

void set_display_cursor(int row, int col) {
  set_vga_cursor(row, col);
}

void shift_display_rows(void) {
  for (int row = 1; row < VGA_ROWS; row++)
    for (int col = 0; col < VGA_COLS; col++)
      *get_cell(row - 1, col) = *get_cell(row, col);

  for (int col = 0; col < VGA_COLS; col++)
    get_cell(VGA_ROWS - 1, col)->chr = 0;
}

void clear_display(void) {
  for (int row = 0; row < VGA_ROWS; row++)
    for (int col = 0; col < VGA_COLS; col++)
      *get_cell(row, col) = (struct vga_cell) { 0, VGA_LIGHT_GRAY, VGA_BLACK };
}
