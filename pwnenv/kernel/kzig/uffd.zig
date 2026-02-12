const std = @import("std");
const lnx = @import("linux.zig");
const util = @import("util.zig");

// handler signature compatible with the original C version.
pub const uffd_handler = *const fn (?*anyopaque) callconv(.c) ?*anyopaque;

pub const uffd_api: u64 = 0xAA;

pub const uffd_event_pagefault: u8 = 0x12;

pub const uffdio_register_mode_missing: u64 = 1 << 0;

pub const uffdio_api = extern struct {
    api: u64,
    features: u64,
    ioctls: u64,
};

pub const uffdio_range = extern struct {
    start: u64,
    len: u64,
};

pub const uffdio_register = extern struct {
    range: uffdio_range,
    mode: u64,
    ioctls: u64,
};

pub const uffdio_copy = extern struct {
    dst: u64,
    src: u64,
    len: u64,
    mode: u64,
    copy: i64,
};

const uffd_msg_pagefault_feat = extern union {
    ptid: u32,
};

const uffd_msg_pagefault = extern struct {
    flags: u64,
    address: u64,
    feat: uffd_msg_pagefault_feat,
};

const uffd_msg_fork = extern struct {
    ufd: u32,
};

const uffd_msg_remap = extern struct {
    from: u64,
    to: u64,
    len: u64,
};

const uffd_msg_remove = extern struct {
    start: u64,
    end: u64,
};

const uffd_msg_reserved = extern struct {
    reserved1: u64,
    reserved2: u64,
    reserved3: u64,
};

const uffd_msg_arg = extern union {
    pagefault: uffd_msg_pagefault,
    fork: uffd_msg_fork,
    remap: uffd_msg_remap,
    remove: uffd_msg_remove,
    reserved: uffd_msg_reserved,
};

pub const uffd_msg = extern struct {
    event: u8,
    reserved1: u8,
    reserved2: u16,
    reserved3: u32,
    arg: uffd_msg_arg,
};

const uffdio_ioctl = struct {
    const type_id: u8 = 0xAA;
    const nr_register: u8 = 0x00;
    const nr_copy: u8 = 0x03;
    const nr_api: u8 = 0x3F;

    const api: u32 = lnx.os.IOCTL.IOWR(type_id, nr_api, uffdio_api);
    const register: u32 = lnx.os.IOCTL.IOWR(type_id, nr_register, uffdio_register);
    const copy: u32 = lnx.os.IOCTL.IOWR(type_id, nr_copy, uffdio_copy);
};

var fault_cnt: u32 = 0;

// Example handler that responds with a dummy page.
pub fn fault_handler_thread_example(arg: ?*anyopaque) callconv(.c) ?*anyopaque {
    const uffd_fd: i32 = @intCast(@intFromPtr(arg.?));

    const map_flags = lnx.os.MAP{ .TYPE = .PRIVATE, .ANONYMOUS = true };
    const dummy_addr = lnx.check(
        lnx.os.mmap(null, 0x1000, lnx.os.PROT.READ | lnx.os.PROT.WRITE, map_flags, -1, 0),
        "mmap(dummy)",
    );
    const dummy_page: [*]u8 = @ptrFromInt(dummy_addr);

    var pfd: lnx.os.pollfd = .{
        .fd = uffd_fd,
        .events = lnx.os.POLL.IN,
        .revents = 0,
    };

    while (true) {
        _ = lnx.check(lnx.os.poll(&pfd, 1, -1), "poll");

        if ((pfd.revents & (lnx.os.POLL.ERR | lnx.os.POLL.HUP)) != 0) {
            std.debug.print("[-] poll: ERR/HUP\n", .{});
            std.process.abort();
        }

        var msg: uffd_msg = undefined;
        const n = lnx.check(
            lnx.os.read(uffd_fd, @as([*]u8, @ptrCast(&msg)), @sizeOf(uffd_msg)),
            "read(uffd)",
        );
        if (n == 0) {
            std.debug.print("[-] read(uffd): EOF\n", .{});
            std.process.abort();
        }

        std.debug.assert(msg.event == uffd_event_pagefault);
        std.debug.print("[ ] page fault occurs\n", .{});
        std.debug.print("      uffd: flag = 0x{x}\n", .{msg.arg.pagefault.flags});
        std.debug.print("      uffd: addr = 0x{x}\n", .{msg.arg.pagefault.address});

        const s = if (fault_cnt == 0) "Hello, World! (1)" else "Hello, World! (2)";
        fault_cnt +%= 1;
        @memcpy(dummy_page[0..s.len], s);
        dummy_page[s.len] = 0;

        var copy: uffdio_copy = .{
            .src = @intFromPtr(dummy_page),
            .dst = msg.arg.pagefault.address & ~@as(u64, 0xfff),
            .len = 0x1000,
            .mode = 0,
            .copy = 0,
        };

        _ = lnx.check(
            lnx.os.ioctl(uffd_fd, uffdio_ioctl.copy, @intFromPtr(&copy)),
            "ioctl(UFFDIO_COPY)",
        );
    }
}

const handler_args = struct {
    handler: uffd_handler,
    uffd_fd: i32,
};

fn handler_entry(args: handler_args) void {
    const p: *anyopaque = @ptrFromInt(@as(usize, @intCast(args.uffd_fd)));
    _ = args.handler(p);
}

// requires `vm.unprivileged_userfaultfd`
pub fn register_uffd(handler: uffd_handler, addr: *anyopaque, len: u64) void {
    std.debug.print(
        "[ ] registering userfaultfd handler to [0x{x}, 0x{x})\n",
        .{ @intFromPtr(addr), @intFromPtr(addr) + len },
    );

    const flags: u32 = @bitCast(lnx.os.O{ .CLOEXEC = true, .NONBLOCK = true });
    const uffd_rc = lnx.check(lnx.os.syscall1(.userfaultfd, @as(usize, flags)), "syscall(userfaultfd)");
    const uffd_fd: i32 = @intCast(uffd_rc);

    var api: uffdio_api = .{
        .api = uffd_api,
        .features = 0,
        .ioctls = 0,
    };
    _ = lnx.check(lnx.os.ioctl(uffd_fd, uffdio_ioctl.api, @intFromPtr(&api)), "ioctl(UFFDIO_API)");

    var reg: uffdio_register = .{
        .range = .{
            .start = @intFromPtr(addr),
            .len = len,
        },
        .mode = uffdio_register_mode_missing,
        .ioctls = 0,
    };
    _ = lnx.check(lnx.os.ioctl(uffd_fd, uffdio_ioctl.register, @intFromPtr(&reg)), "ioctl(UFFDIO_REGISTER)");

    var th = std.Thread.spawn(.{}, handler_entry, .{.{ .handler = handler, .uffd_fd = uffd_fd }}) catch {
        const src = @src();
        util.fatal_at(src.file, src.line, "std.Thread.spawn");
    };
    th.detach();
}
