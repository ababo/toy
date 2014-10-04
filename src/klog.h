#ifndef __KLOG_H_
#define __KLOG_H_

#include <cstdarg>
#include <cstdint>

namespace toy {

class KLog {
 public:
  typedef void (*PutcFunc)(uint32_t chr);

  enum class Level {
    kInfo,
    kWarning,
    kError
  };

  void Initialize(PutcFunc putc, Level level) {
    putc_ = putc; level_ = level;
  }

  int Print(const char* format, va_list vargs);
  int Print(const char* format, ...);

  void Info(const char* format, ...);
  void Warning(const char* format, ...);
  void Error(const char* format, ...);

 private:
  PutcFunc putc_;
  Level level_;
};

extern KLog klog;

}

#endif // __KLOG_H_
