#include "vars.h"

u64 user_cs, user_ss, user_rsp, user_rflags;
u64 kbase = INIT_KBASE;
u64 kheap = INIT_KHEAP;
bool kaslr = INIT_KASLR;
