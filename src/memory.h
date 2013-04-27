#ifndef MEM_MGR_H
#define MEM_MGR_H

#include "util.h"

struct mem_pool { // not thread-safe
  size_t block_size;
};

void *kmalloc(size_t size);
void kfree(void *ptr);

err_code create_mem_pool(size_t block_size, struct mem_pool *pool);
void destroy_mem_pool(struct mem_pool *pool);

void *alloc_block(struct mem_pool *pool);
void free_block(struct mem_pool *pool, void *block);

void init_mem_mgr(void);

#endif // MEM_MGR_H
