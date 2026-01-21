#define _GNU_SOURCE
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "vars.h"

void fatal(const char *str) {
    perror(str);
    exit(1);
}

void dump_buffer(void *const buffer, const uint64_t row) {
    uint64_t *ptr = buffer;
    printf("[ ] buffer dump from %p\n", ptr);
    for (uint64_t i = 0; i < row; i++) {
        printf("      %p|+0x%02lx: 0x%016lx\n", &ptr[i], i * 8, ptr[i]);
    }
}

bool is_valid_kbase(void) {
    if (!kaslr && kbase == default_kbase) {
        return true;
    }
    if (kaslr && ((kbase >> 32) == 0xffffffff && (kbase & 0xfffff) == 0 && default_kbase <= kbase &&
                  kbase <= 0xffffffffc0000000)) {
        return true;
    }
    return false;
}

uint64_t argmin_u64(const uint64_t *const array, const uint64_t len) {
    uint64_t index = 0;
    uint64_t min = UINT64_MAX;
    for (uint64_t i = 0; i < len; i++) {
        if (min > array[i]) {
            min = array[i];
            index = i;
        }
    }
    return index;
}

uint64_t argmax_u64(const uint64_t *const array, const uint64_t len) {
    uint64_t index = 0;
    uint64_t max = 0;
    for (uint64_t i = 0; i < len; i++) {
        if (max < array[i]) {
            max = array[i];
            index = i;
        }
    }
    return index;
}
