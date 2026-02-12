const std = @import("std");
const linux = std.os.linux;
const ctx_mod = @import("context.zig");
const c = @cImport({
    @cInclude("stdio.h");
    @cInclude("stdlib.h");
    @cInclude("unistd.h");
});

pub fn fatal(msg: [*:0]const u8) noreturn {
    c.perror(msg);
    c.exit(1);
    unreachable;
}

pub fn fatal_at(comptime file: []const u8, comptime line: u32, comptime expr: []const u8) noreturn {
    std.debug.print("{s}:{d}: fatal: {s} failed\n", .{ file, line, expr });
    std.process.abort();
}

pub inline fn check(ret: anytype, comptime expr: []const u8) @TypeOf(ret) {
    if (ret == -1) {
        const src = @src();
        fatal_at(src.file, src.line, expr);
    }
    return ret;
}

pub inline fn check_nz(ret: anytype, comptime expr: []const u8) @TypeOf(ret) {
    if (ret != 0) {
        const src = @src();
        fatal_at(src.file, src.line, expr);
    }
    return ret;
}

pub inline fn check_zero(ret: anytype, comptime expr: []const u8) @TypeOf(ret) {
    if (ret == 0) {
        const src = @src();
        fatal_at(src.file, src.line, expr);
    }
    return ret;
}

pub fn dump_buffer(buffer: *const anyopaque, row: u64) void {
    const ptr: [*]const u64 = @ptrCast(@alignCast(buffer));
    std.debug.print("[ ] buffer dump from 0x{x}\n", .{@intFromPtr(ptr)});
    var i: u64 = 0;
    while (i < row) : (i += 1) {
        std.debug.print("      0x{x}|+0x{x:0>3}: 0x{x:0>16}\n", .{
            @intFromPtr(ptr + i),
            i * 8,
            ptr[i],
        });
    }
}

pub fn is_valid_kbase(ctx: *const ctx_mod.ctx) bool {
    return ctx.is_valid_kbase();
}

pub fn argmin_u64(array: []const u64) u64 {
    var index: u64 = 0;
    var min: u64 = std.math.maxInt(u64);

    for (array, 0..) |value, i| {
        if (value < min) {
            min = value;
            index = @intCast(i);
        }
    }

    return index;
}

pub fn argmax_u64(array: []const u64) u64 {
    var index: u64 = 0;
    var max: u64 = 0;

    for (array, 0..) |value, i| {
        if (value > max) {
            max = value;
            index = @intCast(i);
        }
    }

    return index;
}

fn sig_handler(_: i32) callconv(.c) void {
    return;
}

inline fn check_linux(rc: usize, comptime expr: []const u8) usize {
    if (linux.E.init(rc) != .SUCCESS) {
        const src = @src();
        fatal_at(src.file, src.line, expr);
    }
    return rc;
}

pub fn write_cpu_entry_area(payload: [*]u8) void {
    std.debug.print("[ ] writing cpu_entry_area\n", .{});

    const pid = std.posix.fork() catch {
        const src = @src();
        fatal_at(src.file, src.line, "fork");
    };

    // Parent: give the child time to run and return.
    if (pid > 0) {
        std.Thread.sleep(1 * std.time.ns_per_s);
        return;
    }

    var sa: linux.Sigaction = .{
        .handler = .{ .handler = sig_handler },
        .mask = std.mem.zeroes(linux.sigset_t),
        .flags = 0,
    };

    _ = check_linux(linux.sigaction(linux.SIG.TRAP, &sa, null), "sigaction(SIGTRAP)");
    _ = check_linux(linux.sigaction(linux.SIG.SEGV, &sa, null), "sigaction(SIGSEGV)");

    _ = std.posix.setsid() catch {
        const src = @src();
        fatal_at(src.file, src.line, "setsid");
    };

    asm volatile (
        \\movq %[payload], %%rsp
        \\pop %%r15
        \\pop %%r14
        \\pop %%r13
        \\pop %%r12
        \\pop %%rbp
        \\pop %%rbx
        \\pop %%r11
        \\pop %%r10
        \\pop %%r9
        \\pop %%r8
        \\pop %%rax
        \\pop %%rcx
        \\pop %%rdx
        \\pop %%rsi
        \\pop %%rdi
        \\int3
        :
        : [payload] "r" (@as(u64, @intFromPtr(payload))),
        : .{ .memory = true }
    );

    unreachable;
}

pub fn stop_execution(msg: ?[]const u8) void {
    if (msg) |s| {
        std.debug.print("{s}\n", .{s});
    } else {
        std.debug.print("[ ] waiting for input...\n", .{});
    }
    _ = c.getchar();
}
