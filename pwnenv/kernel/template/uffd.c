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
    const i64 uffd = (i64)arg;

    char *const dummy_page =
        mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (unlikely(dummy_page == MAP_FAILED)) {
        fatal("mmap(dummy)");
    }

    struct pollfd pollfd;
    pollfd.fd = uffd;
    pollfd.events = POLLIN;
    while (poll(&pollfd, 1, -1) > 0) {
        if (unlikely(pollfd.revents & POLLERR || pollfd.revents & POLLHUP)) {
            fatal("poll");
        }

        // ページフォルト待機
        static struct uffd_msg msg;
        if (unlikely(read(uffd, &msg, sizeof(msg)) <= 0)) {
            fatal("read(uffd)");
        }
        assert(msg.event == UFFD_EVENT_PAGEFAULT);
        puts("[ ] page fault occurs");
        printf("      uffd: flag = %#llx\n", msg.arg.pagefault.flags);
        printf("      uffd: addr = %#llx\n", msg.arg.pagefault.address);

        // 要求されたページとして返すデータを設定
        static int fault_cnt = 0;
        if (fault_cnt++ == 0) {
            strcpy(dummy_page, "Hello, World! (1)");
        } else {
            strcpy(dummy_page, "Hello, World! (2)");
        }
        struct uffdio_copy copy;
        copy.src = (u64)dummy_page;
        copy.dst = (u64)msg.arg.pagefault.address & ~0xfff;
        copy.len = 0x1000;
        copy.mode = 0;
        copy.copy = 0;
        CHECK(ioctl(uffd, UFFDIO_COPY, &copy));
    }

    return NULL;
}

void register_uffd(void *(*handler)(void *), void *const addr, const u64 len) {
    printf("[ ] registering userfaultfd handler to [%p, %p)\n", addr, addr + len);
    // userfaultfd の作成
    const i64 uffd = CHECK(syscall(__NR_userfaultfd, O_CLOEXEC | O_NONBLOCK));

    // api の設定
    struct uffdio_api uffdio_api;
    uffdio_api.api = UFFD_API;
    uffdio_api.features = 0;
    CHECK(ioctl(uffd, UFFDIO_API, &uffdio_api));

    // ページを userfaultfd に登録
    struct uffdio_register uffdio_register;
    uffdio_register.range.start = (u64)addr;
    uffdio_register.range.len = len;
    uffdio_register.mode = UFFDIO_REGISTER_MODE_MISSING;
    CHECK(ioctl(uffd, UFFDIO_REGISTER, &uffdio_register));

    // ページフォルトを処理するスレッドを作成
    pthread_t th;
    CHECK_NZ(pthread_create(&th, NULL, handler, (void *)uffd));
}
