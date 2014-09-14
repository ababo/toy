#ifndef __STD_STRING_H_
#define __STD_STRING_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void* memset(void* ptr, int value, size_t size);

size_t strlen(const char* str);
char* _strrev(char* str);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __STD_STRING_H_
