#define _GNU_SOURCE
#include <errno.h>
#include <fuse.h>
#include <sched.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "util.h"
#include "vars.h"

static const char *content = "Hello, World!\n";

i32 getattr_callback_example(const char *path, struct stat *stbuf) {
    log_info("getattr_callback_example");
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/pwn") == 0) {
        stbuf->st_mode = S_IFREG | 0777;   // 権限
        stbuf->st_nlink = 1;               // ハードリンクの数
        stbuf->st_size = strlen(content);  // ファイルサイズ
        return 0;
    }

    return -ENOENT;
}

i32 open_callback_example(const char *path, struct fuse_file_info *fi) {
    log_info("open_callback_example");
    (void)path;
    (void)fi;
    return 0;
}

i32 read_callback_example(const char *path, char *buffer, u64 size, off_t offset,
                          struct fuse_file_info *fi) {
    log_info("read_callback_example");
    log_with_prefix("     ", "path  : %s", path);
    log_with_prefix("     ", "size  : %#lx", size);
    log_with_prefix("     ", "offset: %#lx", offset);
    (void)fi;

    if (strcmp(path, "/pwn") == 0) {
        const u64 content_len = strlen(content);
        if (offset < 0) {
            return -EINVAL;
        }
        if ((u64)offset >= content_len) {
            return 0;
        }
        u64 n = content_len - (u64)offset;
        if (n > size) {
            n = size;
        }
        memcpy(buffer, content + offset, n);
        return (i32)n;
    }
    return -ENOENT;
}

static struct fuse_operations fops_example = {
    .getattr = getattr_callback_example,
    .open = open_callback_example,
    .read = read_callback_example,
};

bool fuse_done = false;
void *fuse_thread_example(void *arg) {
    CHECK(mkdir("/tmp/test", 0777));

    struct fuse_args args = FUSE_ARGS_INIT(0, NULL);
    struct fuse_chan *chan = CHECK_NULL(fuse_mount("/tmp/test", &args));

    struct fuse *fuse;
    if (!(fuse = fuse_new(chan, &args, &fops_example, sizeof(fops_example), NULL))) {
        fuse_unmount("/tmp/test", chan);
        fatal("fuse_new");
    }

    cpu_set_t cpu;
    CPU_ZERO(&cpu);
    CPU_SET(0, &cpu);
    CHECK(sched_setaffinity(0, sizeof(cpu_set_t), &cpu));

    fuse_set_signal_handlers(fuse_get_session(fuse));
    fuse_done = true;
    fuse_loop_mt(fuse);

    fuse_unmount("/tmp/test", chan);
    return NULL;
}
