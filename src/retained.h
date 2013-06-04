#ifndef __RETAINED_H
#define __RETAINED_H

struct retained {
  int count;
  void (*free)(void);
};

static inline void retain(void *retained) {
  ((struct retained*)retained)->count++;
}

static inline void release(void *retained) {
  struct retained *r = retained;
  if (!--r->count)
    r->free();
}

#endif // __RETAINED_H
