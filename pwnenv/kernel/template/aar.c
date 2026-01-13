#include <assert.h>
#include <linux/prctl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/prctl.h>

#include "util.h"

static bool found_target(void *const addr, void *const target, const uint64_t len,
                         uint32_t (*aar32)(void *), uint64_t (*aar64)(void *)) {
    if (aar32 != NULL) {
        for (uint64_t offset = 0; offset < len; offset += 4) {
            uint32_t actual;
            uint32_t expect;
            if (offset + 4 < len) {
                actual = aar32(addr);
                expect = *(uint32_t *)(target + offset);
            } else {
                uint64_t rest = len - offset;
                actual = aar32(addr);
                expect = (*(uint32_t *)(target + offset)) & ((1 << (8 * rest)) - 1);
            }
            if (actual != expect) {
                return false;
            }
        }
        return true;
    }
    if (aar64 != NULL) {
        for (uint64_t offset = 0; offset < len; offset += 8) {
            uint64_t actual;
            uint64_t expect;
            if (offset + 8 < len) {
                actual = aar64(addr);
                expect = *(uint64_t *)(target + offset);
            } else {
                uint64_t rest = len - offset;
                actual = aar32(addr);
                expect = (*(uint64_t *)(target + offset)) & ((1 << (8 * rest)) - 1);
            }
            if (actual != expect) {
                return false;
            }
        }
        return true;
    }
    return false;
}

void *find_target_from_heap(void *const addr_start, void *const target, const uint64_t len,
                            uint32_t (*aar32)(void *), uint64_t (*aar64)(void *)) {
    uint64_t addr;
    if (aar32 != NULL) {
        for (addr = (uint64_t)addr_start;; addr += 8) {
            if ((addr & 0xffff) == 0) {
                printf("[ ] searching %p...\n", (void *)addr);
            }
            if (found_target((void *)addr, target, len, aar32, NULL)) {
                printf("[+] found target at %p\n", (void *)addr);
                return (void *)addr;
            }
        }
    }
    if (aar64 != NULL) {
        for (addr = (uint64_t)addr_start;; addr += 8) {
            if ((addr & 0xffff) == 0) {
                printf("[ ] searching %p...\n", (void *)addr);
            }
            if (found_target((void *)addr, target, len, NULL, aar64)) {
                printf("[+] found target at %p\n", (void *)addr);
                return (void *)addr;
            }
        }
    }
    puts("[-] please implement aar function");
    return NULL;
}

void *find_comm(void *const addr_start, uint32_t (*aar32)(void *), uint64_t (*aar64)(void *)) {
    char name[] = "maimai";
    assert(sizeof(name) <= 16);
    if (prctl(PR_SET_NAME, name) == -1) {
        fatal("prctl");
    }
    printf("[ ] start searching comm from %p\n", addr_start);
    return find_target_from_heap(addr_start, name, sizeof(name), aar32, aar64);
}
