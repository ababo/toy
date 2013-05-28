#ifndef __MEMORY_H
#define __MEMORY_H

#include "common.h"

struct mem_pool { // not thread-safe
  size_t block_size;
};

void *kmalloc(size_t size);
void kfree(void *ptr);

err_code create_mem_pool(size_t block_size, struct mem_pool *pool);
void destroy_mem_pool(struct mem_pool *pool);

void *alloc_block(struct mem_pool *pool);
void free_block(struct mem_pool *pool, void *block);

void init_memory(void);

#endif // __MEMORY_H
