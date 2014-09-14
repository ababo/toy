#ifndef __STD_STDINT_H_
#define __STD_STDINT_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#ifdef __LP64__
typedef long int64_t;
typedef unsigned long uint64_t;
typedef unsigned long size_t;
#else
typedef long long int64_t;
typedef unsigned long long uint64_t;
typedef unsigned int size_t;
#endif // __LP64__

#define INT8_MAX 0x7F
#define INT8_MIN (-INT8_MAX - 1)
#define INT16_MAX 0x7FFF
#define INT16_MIN (-INT16_MAX - 1)
#define INT32_MAX 0x7FFFFFFF
#define INT32_MIN (-INT32_MAX - 1)
#define INT64_MAX 0x7FFFFFFFFFFFFFFFL
#define INT64_MIN (-INT64_MAX - 1L)
#define UINT8_MAX (uint8_t)0xFF
#define UINT16_MAX (uint16_t)0xFFFF
#define UINT32_MAX (uint32_t)0xFFFFFFFF
#define UINT64_MAX (uint64_t)0xFFFFFFFFFFFFFFFFL

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __STD_STDINT_H_
