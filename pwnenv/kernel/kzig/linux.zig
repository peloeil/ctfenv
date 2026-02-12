const std = @import("std");
const util = @import("util.zig");

pub const os = std.os.linux;

// prctl(2)
pub const pr_set_name: i32 = 15;

pub inline fn check(rc: usize, comptime expr: []const u8) usize {
    if (os.E.init(rc) != .SUCCESS) {
        const src = @src();
        util.fatal_at(src.file, src.line, expr);
    }
    return rc;
}
