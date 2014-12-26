#include "klog.h"

int klog_vprintf(klog_putc putc, void *putc_arg,
                 const char *format, va_list vargs) {
  char chr, *str, buf[20];
  int num = 0;
  long lint;

  struct {
    unsigned size_long : 1;
  } attrs;

  while ((chr = *format++))
    if (chr == '%') {
      memset(&attrs, 0, sizeof(attrs));

next_attr:
      switch ((chr = *format++)) {
        case 'l':
          attrs.size_long = true;
          goto next_attr;

        case '%':
          putc(putc_arg, '%');
          num++;
          break;

        case 's':
          str = va_arg(vargs, char*);
puts:
          while (*str) {
            putc(putc_arg, *str++);
            num++;
          }
          break;

        case 'd':
          lint = va_arg(vargs, long);
          if (!attrs.size_long) lint = (int)lint;
          if (lint < 0) {
            putc(putc_arg, '-');
            lint = -lint;
            num++;
          }
          str = ultoa(lint, buf, 10);
          goto puts;

        case 'x':
        case 'X':
          lint = va_arg(vargs, long);
          if (!attrs.size_long) lint = (unsigned int)lint;
          str = ultoa(lint, buf, chr == 'x' ? 16 : -16);
          goto puts;

        case 'c':
          buf[0] = (char)va_arg(vargs, int);
          buf[1] = 0;
          str = buf;
          goto puts;

          /* TODO: implement other types and attributes */
      }
    }
    else {
      putc(putc_arg, chr);
      ++num;
    }

  return num;
}

int klog_printf(klog_putc putc, void *putc_arg,
                const char *format, ...) {
  va_list vargs;
  va_start(vargs, format);
  int count = klog_vprintf(putc, putc_arg, format, vargs);
  va_end(vargs);
  return count;
}

static klog_putc putc_;
static void *putc_arg_;
static enum klog_level level_;

void klog_init(klog_putc putc, void *putc_arg, enum klog_level level) {
  putc_ = putc;
  putc_arg_ = putc_arg;
  level_ = level;
}

static const char *const PREFIXES[] = { "i ", "W ", "E " };

bool klog(enum klog_level level, const char *format, ...) {
  /* TODO: add thread-safery */
  if (level_ > level) return false;

  va_list vargs;
  va_start(vargs, format);
  klog_printf(putc_, putc_arg_, PREFIXES[level]);
  klog_vprintf(putc_, putc_arg_, format, vargs);
  putc_(putc_arg_, '\n');
  va_end(vargs);

  return true;
}
