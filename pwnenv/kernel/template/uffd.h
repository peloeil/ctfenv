#ifndef __KPWN_UFFD_H
#define __KPWN_UFFD_H

#include <assert.h>
#include <linux/userfaultfd.h>
#include <poll.h>
#include <unistd.h>

#include "types.h"

// requires `vm.unprivileged_userfaultfd`
void register_uffd(void *(*handler)(void *), void *const addr, const u64 len);

#endif  //__KPWN_UFFD_H
