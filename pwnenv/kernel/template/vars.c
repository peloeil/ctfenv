#include "vars.h"

uint64_t user_cs, user_ss, user_rsp, user_rflags;
uint64_t kbase = INIT_KBASE;
uint64_t kheap = INIT_KHEAP;
bool kaslr = INIT_KASLR;
