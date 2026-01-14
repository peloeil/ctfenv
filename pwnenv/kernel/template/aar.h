#ifndef __KPWN_AAR_H
#define __KPWN_AAR_H

#include <stdint.h>

void *find_target_from_heap(void *const addr_start, void *const target, const uint64_t len,
                            uint32_t (*aar32)(void *), uint64_t (*aar64)(void *));
void *find_comm(void *const addr_start, uint32_t (*aar32)(void *), uint64_t (*aar64)(void *));
void *comm_to_addr_cred(void *addr_comm, uint32_t (*aar32)(void *), uint64_t (*aar64)(void *));
void dump_buffer_aar(void *addr_start, const uint64_t row, uint32_t (*aar32)(void *),
                     uint64_t (*aar64)(void *));

#endif  // __KPWN_AAR_H
