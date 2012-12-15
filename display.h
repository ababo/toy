#ifndef DISPLAY_H
#define DISPLAY_H

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

#define ROWS_NUMBER 80
#define COLUMNS_NUMBER 25

void set_foreground (int color);
void set_background (int color);
void set_caret (int row, int column);
void putc (char chr);

#endif /* DISPLAY_H */
