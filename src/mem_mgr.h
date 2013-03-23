#ifndef MEM_MGR_H
#define MEM_MGR_H

#include "util.h"

void *kmalloc(size_t size);
void kfree(void *ptr);

void init_mem_mgr(void);

#endif // MEM_MGR_H
