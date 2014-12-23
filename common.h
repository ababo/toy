/*
 *Common kernel definitions.
 */

#ifndef __COMMON_H_
#define __COMMON_H_

/* from stdint.h */
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

#endif /* __COMMON_H_ */
