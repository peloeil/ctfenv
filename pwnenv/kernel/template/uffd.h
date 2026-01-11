#ifndef __KPWN_UFFD_H
#define __KPWN_UFFD_H

#include <stdint.h>

void register_uffd(void *(*handler)(void *), void *const addr, const uint64_t len);

#endif  //__KPWN_UFFD_H
