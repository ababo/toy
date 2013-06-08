#ifndef __X86_64_AHCI_H
#define __X86_64_AHCI_H

#include "../driver.h"

struct ahci_driver {
  struct storage_driver storage_driver;

};

const struct ahci_driver *get_ahci_driver(void);

void init_ahci(void); // not thread-safe
void dispose_ahci(void); // not thread-safe

#endif // __X86_64_AHCI_H
