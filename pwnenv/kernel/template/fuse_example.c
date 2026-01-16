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

int32_t getattr_callback_example(const char *path, struct stat *stbuf) {
    puts("[ ] getattr_callback_example");
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/pwn") == 0) {
        stbuf->st_mode = S_IFREG | 0777;   // 権限
        stbuf->st_nlink = 1;               // ハードリンクの数
        stbuf->st_size = strlen(content);  // ファイルサイズ
        return 0;
    }

    return -ENOENT;
}

int32_t open_callback_example(const char *path, struct fuse_file_info *fi) {
    puts("[ ] open_callback_example");
    return 0;
}

int32_t read_callback_example(const char *path, char *buffer, uint64_t size, off_t offset,
                              struct fuse_file_info *fi) {
    puts("[ ] read_callback_example");
    printf("      path  : %s\n", path);
    printf("      size  : 0x%lx\n", size);
    printf("      offset: 0x%lx\n", offset);

    if (strcmp(path, "/pwn") == 0) {
        memcpy(buffer, "Hello, World!", 14);
        return size;
    }
    return 0;
}

static struct fuse_operations fops_example = {
    .getattr = getattr_callback_example,
    .open = open_callback_example,
    .read = read_callback_example,
};

bool fuse_done = false;
void *fuse_thread_example(void *arg) {
    CHECK_NZ(mkdir("/tmp/test", 0777));

    struct fuse_args args = FUSE_ARGS_INIT(0, NULL);
    struct fuse_chan *chan = CHECK_NULL(fuse_mount("/tmp/test", &args));

    struct fuse *fuse;
    if (!(fuse = fuse_new(chan, &args, &fops_example, sizeof(fops_example), NULL))) {
        fuse_unmount("/tmp/test", chan);
        fatal("fuse_new");
    }

    CHECK_NZ(sched_setaffinity(0, sizeof(cpu_set_t), &t1_cpu));

    fuse_set_signal_handlers(fuse_get_session(fuse));
    fuse_done = true;
    fuse_loop_mt(fuse);

    fuse_unmount("/tmp/test", chan);
    return NULL;
}
