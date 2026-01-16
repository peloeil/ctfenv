#ifndef __KPWN_UTIL_H
#define __KPWN_UTIL_H

#include <stdbool.h>
#include <stdint.h>

void fatal(const char *str);
void dump_buffer(void *const buffer, const uint64_t row);
bool is_valid_kbase(void);

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#endif  //__KPWN_UTIL_H
