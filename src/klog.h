#ifndef __KLOG_H_
#define __KLOG_H_

#include <cstdarg>
#include <cstdint>

namespace toy {

enum class KLogLevel { kInfo, kWarning, kError };

class KLog {
 public:
  typedef void (*PutcFunc)(uint32_t chr);

  void Initialize(PutcFunc putc, KLogLevel level) {
    putc_ = putc; level_ = level;
  }

  int Print(const char* format, va_list vargs);
  int Print(const char* format, ...);

  void Info(const char* format, ...);
  void Warning(const char* format, ...);
  void Error(const char* format, ...);

 private:
  PutcFunc putc_;
  KLogLevel level_;
};

extern KLog klog;

}

#endif // __KLOG_H_
