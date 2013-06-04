#define PANIC(format, ...) {                    \
    LOG(PANIC, format, ##__VA_ARGS__);          \
    ASMV("cli; jmp __halt");                    \
  }
