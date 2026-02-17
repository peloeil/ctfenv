#ifndef __KPWN_RET2USR_H
#define __KPWN_RET2USR_H

#include "types.h"

void spawn_shell(void);
void save_state(void);
void krop(u64 *ptr);

// requires nosmep
void restore_state(void);

// requires nosmep
void escalate_privilege(void);

#endif  // __KPWN_RET2USR_H
