#ifndef __KPWN_UTIL_H
#define __KPWN_UTIL_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"

void fatal(const char *str);
void dump_buffer(void *const buffer, const u64 row);
bool is_valid_kbase(void);
u64 argmin_u64(const u64 *const array, const u64 len);
u64 argmax_u64(const u64 *const array, const u64 len);
void write_cpu_entry_area(char *payload);
void stop_execution(void);
void assign_cpu(const u64 core);

__attribute__((noreturn)) static inline void fatal_at(const char *file, int line,
                                                      const char *expr) {
    int saved_errno = errno;

    if (saved_errno) {
        fprintf(stderr, "%s:%d: fatal: %s failed (errno=%d: %s)\n", file, line, expr, saved_errno,
                strerror(saved_errno));
    } else {
        fprintf(stderr, "%s:%d: fatal: %s failed\n", file, line, expr);
    }
    abort();
}

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define _CHECK_IMPL(expr, err_cond)              \
    ({                                           \
        __typeof__(expr) __ret = (expr);         \
        if (unlikely(__ret err_cond))            \
            fatal_at(__FILE__, __LINE__, #expr); \
        __ret;                                   \
    })
#define CHECK(expr) _CHECK_IMPL(expr, == ((__typeof__(expr))-1))
#define CHECK_NZ(expr) _CHECK_IMPL(expr, != 0)
#define CHECK_ZERO(expr) _CHECK_IMPL(expr, == 0)
#define CHECK_NULL(expr) _CHECK_IMPL(expr, == NULL)
#define CHECK_NEG(expr) _CHECK_IMPL(expr, < 0)

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a < b ? b : a)
#define FABS(a) (a > 0 ? a : -a)
#define DECLARE_REG(reg) register u64 reg##_val asm(#reg)

#endif  //__KPWN_UTIL_H
