#ifndef __KPWN_UTIL_H
#define __KPWN_UTIL_H

#include <stdbool.h>
#include <stdint.h>

void fatal(const char *str);
void dump_buffer(void *const buffer, const uint64_t row);
bool is_valid_kbase(void);

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define _CHECK_IMPL(expr, err_cond)      \
    ({                                   \
        __typeof__(expr) __ret = (expr); \
        if (unlikely(__ret err_cond))    \
            fatal(#expr);                \
        __ret;                           \
    })
#define CHECK(expr) _CHECK_IMPL(expr, == -1)
#define CHECK_NZ(expr) _CHECK_IMPL(expr, != 0)
#define CHECK_ZERO(expr) _CHECK_IMPL(expr, == 0)
#define CHECK_NULL(expr) _CHECK_IMPL(expr, == NULL)
#define CHECK_NEG(expr) _CHECK_IMPL(expr, < 0)

#endif  //__KPWN_UTIL_H
