#ifndef __KPWN_UTIL_H
#define __KPWN_UTIL_H

#include <stdint.h>
#include <stdbool.h>

void fatal(const char *str);
void dump_buffer(void *const buffer, const uint64_t row);
bool valid_kbase(void);

#endif  //__KPWN_UTIL_H
