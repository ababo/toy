#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "util.h"

typedef void (*intr_handler) (void);

void init_interrupts (void);

intr_handler get_intr_handler (int intr);
void set_intr_handler (int intr, intr_handler handler);

#endif // INTERRUPT_H
