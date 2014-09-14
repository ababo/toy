#ifndef __STD_STDARG_H_
#define __STD_STDARG_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef __builtin_va_list va_list;

#define va_start(vargs, last_param) __builtin_va_start(vargs, last_param)
#define va_arg(vargs, arg_type) __builtin_va_arg(vargs, arg_type)
#define va_end(vargs) __builtin_va_end(vargs)

#ifdef __cplusplus
}
#endif

#endif // __STD_STDARG_H_
