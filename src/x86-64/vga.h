#ifndef X86_64_VGA_H
#define X86_64_VGA_H

#include "../display.h" // vga.c implements this API

#define VGA_BLACK 0
#define VGA_LOW_BLUE 1
#define VGA_LOW_GREEN 2
#define VGA_LOW_CYAN 3
#define VGA_LOW_RED 4
#define VGA_LOW_MAGENTA 5
#define VGA_BROWN 6
#define VGA_LIGHT_GRAY 7
#define VGA_DARK_GRAY 8
#define VGA_HIGH_BLUE 9
#define VGA_HIGH_GREEN 10
#define VGA_HIGH_CYAN 11
#define VGA_HIGH_RED 12
#define VGA_HIGH_MAGENTA 13
#define VGA_HIGH_YELLOW 14
#define VGA_WHITE 15

#define VGA_ROWS 25
#define VGA_COLS 80

void get_vga_cell(int row, int col, char *chr, int *fcolor, int *bcolor);
void set_vga_cell(int row, int col, char chr, int fcolor, int bcolor);

void get_vga_cursor(int *row, int *col);
void set_vga_cursor(int row, int col);

struct spinlock *get_vga_lock(void);

void init_vga(void);

#endif // X86_64_VGA_H
