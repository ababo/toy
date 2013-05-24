#ifndef DISPLAY_H
#define DISPLAY_H

// portable display API

struct spinlock;

void get_display_size(int *rows, int *cols);
struct spinlock *get_display_lock(void);

void set_display_char(int row, int col, char chr);
void set_display_cursor(int row, int col);

void shift_display_rows(void);
void clear_display(void);

#endif // DISPLAY_H
