#include "memory.h"
#include "config.h"
#include "sync.h"

/// TODO: Replace this dummy code with a real implementaion

static struct spinlock lock;
static uint8_t *next_free;

void *kmalloc(size_t size) {
  acquire_spinlock(&lock, 0);
  void *ptr = next_free;
  next_free += size;
  release_spinlock(&lock);
  return ptr;
}

void kfree(UNUSED void *ptr) {
}

err_code create_mem_pool(size_t block_size, struct mem_pool *pool) {
  pool->block_size = block_size;
  return ERR_NONE;
}

void destroy_mem_pool(UNUSED struct mem_pool *pool) {
}

void *alloc_block(struct mem_pool *pool) {
  return kmalloc(pool->block_size);
}

void free_block(UNUSED struct mem_pool *pool, UNUSED void *block) {
}

void __map_memory(void);

void init_memory(void) {
  extern int lds_kernel_size;
  create_spinlock(&lock);
  next_free = (uint8_t*)(CONFIG_KERNEL_ADDR + (uint64_t)&lds_kernel_size);
  __map_memory();
  LOG_DEBUG("done");
}
