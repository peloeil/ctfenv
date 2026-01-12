#ifndef __KPWN_UFFD_H
#define __KPWN_UFFD_H

#include <assert.h>
#include <linux/userfaultfd.h>
#include <poll.h>
#include <stdint.h>
#include <unistd.h>

void register_uffd(void *(*handler)(void *), void *const addr, const uint64_t len);

#endif  //__KPWN_UFFD_H
