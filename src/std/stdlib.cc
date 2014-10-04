#include "stdlib.h"

#include <string.h>

#include "klog.h"

using namespace toy;

extern "C" {

char* ultoa(unsigned long value, char* buf, int radix) {
  char* str = buf, lbase;
  if (radix > 0) lbase = 'a';
  else lbase = 'A', radix = -radix;

  if (radix < 2 || radix > 36) {
    *buf = 0;
    return buf;
  }

  lbase -= 10;
  do {
    int rem = value % radix;
    *buf++ = rem + (rem < 10 ? '0' : lbase);
  }
  while (value /= radix);

  *buf = 0;
  return _strrev(str);
}

void __halt(void);

void abort(void) {
  klog.Error("The system is stopped: 'abort' was called");
  __halt();
}

}

