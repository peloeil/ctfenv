#include <assert.h>
#include <fcntl.h>
#include <linux/userfaultfd.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "util.h"

void *fault_handler_thread_example(void *arg) {
    const int64_t uffd = (int64_t)arg;

    char *const dummy_page =
        mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (dummy_page == MAP_FAILED) {
        fatal("mmap(dummy)");
    }

    struct pollfd pollfd;
    pollfd.fd = uffd;
    pollfd.events = POLLIN;
    while (poll(&pollfd, 1, -1) > 0) {
        if (pollfd.revents & POLLERR || pollfd.revents & POLLHUP) {
            fatal("poll");
        }

        // ページフォルト待機
        static struct uffd_msg msg;
        if (read(uffd, &msg, sizeof(msg)) <= 0) {
            fatal("read(uffd)");
        }
        assert(msg.event == UFFD_EVENT_PAGEFAULT);
        puts("[ ] page fault occurs");
        printf("[ ]   uffd: flag=%p\n", (void *)msg.arg.pagefault.flags);
        printf("[ ]   uffd: addr=%p\n", (void *)msg.arg.pagefault.address);

        // 要求されたページとして返すデータを設定
        static int fault_cnt = 0;
        if (fault_cnt++ == 0) {
            strcpy(dummy_page, "Hello, World! (1)");
        } else {
            strcpy(dummy_page, "Hello, World! (2)");
        }
        struct uffdio_copy copy;
        copy.src = (uint64_t)dummy_page;
        copy.dst = (uint64_t)msg.arg.pagefault.address & ~0xfff;
        copy.len = 0x1000;
        copy.mode = 0;
        copy.copy = 0;
        if (ioctl(uffd, UFFDIO_COPY, &copy) == -1) {
            fatal("ioctl(UFFDIO_COPY)");
        }
    }

    return NULL;
}

void register_uffd(void *(*handler)(void *), void *const addr, const uint64_t len) {
    // userfaultfd の作成
    const int64_t uffd = syscall(__NR_userfaultfd, O_CLOEXEC | O_NONBLOCK);
    if (uffd == -1) {
        fatal("userfaultfd");
    }

    // api の設定
    struct uffdio_api uffdio_api;
    uffdio_api.api = UFFD_API;
    uffdio_api.features = 0;
    if (ioctl(uffd, UFFDIO_API, &uffdio_api) == -1) {
        fatal("ioctl(UFFDIO_API)");
    }

    // ページを userfaultfd に登録
    struct uffdio_register uffdio_register;
    uffdio_register.range.start = (uint64_t)addr;
    uffdio_register.range.len = len;
    uffdio_register.mode = UFFDIO_REGISTER_MODE_MISSING;
    if (ioctl(uffd, UFFDIO_REGISTER, &uffdio_register) == -1) {
        fatal("UFFDIO_REGISTER");
    }

    // ページフォルトを処理するスレッドを作成
    pthread_t th;
    if (pthread_create(&th, NULL, handler, (void *)uffd)) {
        fatal("pthread_create");
    }
}
