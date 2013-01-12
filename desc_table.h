#ifndef DESC_TABLE_H
#define DESC_TABLE_H

#include "util.h"

#define INT_VECTOR_NUMBER 32

static inline bool is_int_reserved(int vector) {
  return vector == 15 || (vector >= 20 && vector <= 31);
}
static inline bool is_int_error(int vector) {
  return vector == 8 || (vector >= 10 && vector <= 14) || vector == 17;
}

void init_desc_tables(void);

void *get_isr(int vector);
void set_isr(int vector, void *isr);

#endif // DESC_TABLE_H
