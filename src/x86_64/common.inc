#define PANIC(format, ...) {                    \
    LOG(PANIC, format, ##__VA_ARGS__);          \
    ASMV("cli; jmp __halt");                    \
  }

#define PACKED_POINTER_DATA_BITS 12
