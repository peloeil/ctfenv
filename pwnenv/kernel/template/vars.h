#ifndef __KPWN_VARS_H
#define __KPWN_VARS_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.h"

/* ------------------- uneditable ------------------- */
#define DUMMY_VALUE (0xddddddddddddddddull)
#define default_kbase (0xffffffff81000000ull)
#define kbase_offset (kbase - default_kbase)
#define PAGE_SIZE (0x1000)
extern u64 user_cs, user_ss, user_rsp, user_rflags;
extern u64 kbase;
extern u64 kheap;

/* -------------------- editable -------------------- */
#define CONFIG_PHYSICAL_START (0x1000000ull)
#define CONFIG_PHYSICAL_ALIGN (0x200000ull)
#define CONFIG_RANDOMIZE_BASE_MAX_OFFSET (0x40000000ull)
#define PUD_SIZE (0x40000000ull)
#define PAGE_SIZE (0x1000ull)
#define PAGE_OFFSET_BASE_LOWER_BOUND (0xffff888000000000ull)
#define PAGE_OFFSET_BASE_UPPER_BOUND (0xfffffe0000000000ull)

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
#define add_rsp_0x140_pop6_ret (DUMMY_VALUE + kbase_offset)

// global variable initial values
#define INIT_KBASE (default_kbase)
#define INIT_KHEAP (0ull)

#endif  // __KPWN_VARS_H
