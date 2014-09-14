#include "klog.h"

#include <cstdlib>
#include <cstring>

namespace toy {

int KLog::Print(const char* format, va_list vargs) {
  int num = 0;
  char chr, *str, buf[20];
  long lint;

  struct {
    uint32_t size_long : 1;
  } attrs;

  while ((chr = *format++))
    if (chr == '%') {
      memset(&attrs, 0, sizeof(attrs));

next_attr:
      switch ((chr = *format++)) {
        case 'l': {
          attrs.size_long = true;
          goto next_attr;
        }

        case '%': {
          putc_('%');
          num++;
          break;
        }

        case 's': {
          str = va_arg(vargs, char*);
puts:
          while (*str) {
            putc_(*str++);
            num++;
          }
          break;
        }

        case 'd': {
          lint = va_arg(vargs, long);
          if (!attrs.size_long) {
            lint = static_cast<int>(lint);
          }
          if (lint < 0) {
            putc_('-');
            lint = -lint;
            num++;
          }
          str = ultoa(lint, buf, 10);
          goto puts;
        }

        case 'x':
        case 'X': {
          lint = va_arg(vargs, long);
          if (!attrs.size_long) {
            lint = static_cast<unsigned int>(lint);
          }
          str = ultoa(lint, buf, chr == 'x' ? 16 : -16);
          goto puts;
        }

        case 'c': {
          buf[0] = static_cast<char>(va_arg(vargs, int));
          buf[1] = 0;
          str = buf;
          goto puts;
        }

        // TODO: implement other types and attributes
      }
    }
    else {
      putc_(chr);
      num++;
    }

  return num;
}

int KLog::Print(const char* format, ...) {
  va_list vargs;
  va_start(vargs, format);
  int result = Print(format, vargs);
  va_end(vargs);
  return result;
}

void KLog::Info(const char* format, ...) {
  if (level_ > KLogLevel::kInfo)
    return;
  va_list vargs;
  va_start(vargs, format);
  Print("i ");
  Print(format, vargs);
  putc_('\n');
  va_end(vargs);
}

void KLog::Warning(const char* format, ...) {
  if (level_ > KLogLevel::kWarning)
    return;
  va_list vargs;
  va_start(vargs, format);
  Print("W ");
  Print(format, vargs);
  putc_('\n');
  va_end(vargs);
}

void KLog::Error(const char* format, ...) {
  va_list vargs;
  va_start(vargs, format);
  Print("E ");
  Print(format, vargs);
  putc_('\n');
  va_end(vargs);
}

KLog klog;

}
