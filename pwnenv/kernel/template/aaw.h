#ifndef __KPWN_AAW_H
#define __KPWN_AAW_H

#include "types.h"

void overwrite_cred(void *const addr_cred, void (*aaw32)(void *, u32), void (*aaw64)(void *, u64));

// requires `CONFIG_STATIC_USERMODEHELPER=n`
void overwrite_modprobe_path(const char *const cmd, void (*aaw32)(void *, u32),
                             void (*aar64)(void *, u64));

#endif  //__KPWN_AAW_H
