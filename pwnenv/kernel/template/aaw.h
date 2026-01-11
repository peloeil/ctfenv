#ifndef __KPWN_AAW_H
#define __KPWN_AAW_H

#include <stdint.h>

void overwrite_modprobe_path(const char *const cmd, void (*aaw32)(void *, uint32_t),
                             void (*aar64)(void *, uint64_t));
void overwrite_cred(void *const addr_cred, void (*aaw32)(void *, uint32_t),
                    void (*aaw64)(void *, uint64_t));

#endif  //__KPWN_AAW_H
