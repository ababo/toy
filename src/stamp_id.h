#ifndef __STAMP_ID_H
#define __STAMP_ID_H

#include "common.h"

#define WITH_STAMP_ID(type, var, id, magic_)    \
  int stamp = get_stamp_id_stamp(id);           \
  type *var = get_stamp_id_ptr(id);             \
  if (var->magic != magic_)                     \
    err = ERR_NOT_FOUND;                        \
  else if (var->stamp != stamp)                 \
    err = ERR_EXPIRED;                          \
  else

#define STAMP_ID_STAMP_BITS 12

typedef uint64_t stamp_id;

static inline stamp_id get_stamp_id(const void *ptr, int stamp) {
  return (uint64_t)ptr | ((uint64_t)stamp << (64 - STAMP_ID_STAMP_BITS));
}

static inline void *get_stamp_id_ptr(stamp_id id) {
  return (void*) (id << STAMP_ID_STAMP_BITS >> STAMP_ID_STAMP_BITS);
}

static inline int get_stamp_id_stamp(stamp_id id) {
  return (int)(id >> (64 - STAMP_ID_STAMP_BITS));
}

static inline void assign_stamp_id_stamp(int *stamp, int *global_stamp) {
  *global_stamp = ((*stamp = *global_stamp) + 1) % (1 << STAMP_ID_STAMP_BITS);
}

#endif // __STAMP_ID_H
