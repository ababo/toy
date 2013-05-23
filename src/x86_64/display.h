#ifndef X86_64_DISPLAY_H
#define X86_64_DISPLAY_H

#include "spinlock.h"

// simple printing support
int get_display_rows(void);
int get_display_cols(void);
void set_display_char(int row, int col, char chr);
void set_display_cursor(int row, int col);
void shift_display_rows(void);
struct spinlock *get_display_lock(void);

#endif // X86_64_DISPLAY_H
