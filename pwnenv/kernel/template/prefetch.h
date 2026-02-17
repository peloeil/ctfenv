#ifndef __KPWN_PREFETCH_H
#define __KPWN_PREFETCH_H

#include "types.h"

// requires `-enable-kvm`
u64 prefetch_kbase();

#endif  //__KPWN_PREFETCH_H
