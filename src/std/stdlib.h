#ifndef __STD_STDLIB_H_
#define __STD_STDLIB_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

char* ultoa(unsigned long value, char* buf, int radix);

void abort(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __STD_STDLIB_H_
