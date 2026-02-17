#ifndef __KPWN_AAR_H
#define __KPWN_AAR_H

#include "types.h"

void *find_target_from_heap(void *const addr_start, void *const target, const u64 len,
                            u32 (*aar32)(void *), u64 (*aar64)(void *));
void *find_comm(void *const addr_start, u32 (*aar32)(void *), u64 (*aar64)(void *));
void *comm_to_addr_cred(void *addr_comm, u32 (*aar32)(void *), u64 (*aar64)(void *));
void dump_buffer_aar(void *addr_start, const u64 row, u32 (*aar32)(void *), u64 (*aar64)(void *));

#endif  // __KPWN_AAR_H
