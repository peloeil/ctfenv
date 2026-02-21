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

void dump_buffer(void *const buffer, const u64 row) {
    u64 *ptr = buffer;
    printf("[ ] buffer dump from %p\n", ptr);
    for (u64 i = 0; i < row; i++) {
        u8 ascii[9] = {0};
        *(u64 *)ascii = ptr[i];
        for (u64 j = 0; j < 8; j++) {
            if (ascii[j] < 0x20 || ascii[j] == 0x7f) {
                ascii[j] = '.';
            }
        }
        printf("      %p|+0x%03lx: 0x%016lx  |%s|\n", &ptr[i], i * 8, ptr[i], ascii);
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

u64 argmin_u64(const u64 *const array, const u64 len) {
    u64 index = 0;
    u64 min = UINT64_MAX;
    for (u64 i = 0; i < len; i++) {
        if (min > array[i]) {
            min = array[i];
            index = i;
        }
    }
    return index;
}

u64 argmax_u64(const u64 *const array, const u64 len) {
    u64 index = 0;
    u64 max = 0;
    for (u64 i = 0; i < len; i++) {
        if (max < array[i]) {
            max = array[i];
            index = i;
        }
    }
    return index;
}

static void sig_handler(i32 s) {
    (void)s;
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

void assign_cpu(const u64 core) {
    cpu_set_t cpu;
    CPU_ZERO(&cpu);
    CPU_SET(core, &cpu);
    CHECK(sched_setaffinity(0, sizeof(cpu_set_t), &cpu));
}
