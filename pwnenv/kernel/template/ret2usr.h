#ifndef __KPWN_RET2USR_H
#define __KPWN_RET2USR_H

#include <stdint.h>

void win(void);
void save_state(void);
void restore_state(void);
void escalate_privilege(void);
void krop(uint64_t *ptr);

#endif  // __KPWN_RET2USR_H
