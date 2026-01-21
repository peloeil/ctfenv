#ifndef __KPWN_PREFETCH_H
#define __KPWN_PREFETCH_H

#include <stdint.h>

// requires `-enable-kvm`
uint64_t prefetch_kbase();

#endif  //__KPWN_PREFETCH_H
