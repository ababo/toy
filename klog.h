/*
 * Kernel log facility.
 */

#ifndef __KLOG_H_
#define __KLOG_H_

#include "std.h"

/* abstract character putter */
typedef void (*klog_putc)(void *arg, uint32_t chr);

/* generic formatting (not thread-safe) */
int klog_vprintf(klog_putc putc, void *putc_arg,
                 const char *format, va_list vargs);
int klog_printf(klog_putc putc, void *putc_arg,
                const char *format, ...);

enum klog_level { INFO, WARNING, ERROR };

/* global kernel logger */
void klog_init(klog_putc putc, void *putc_arg, enum klog_level level);
bool klog(enum klog_level level, const char *format, ...); /* thread-safe */

#endif /* __KLOG_H_ */
