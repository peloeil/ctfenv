const sym = @import("sym.zig");

pub const ctx = struct {
    user_cs: u64 = 0,
    user_ss: u64 = 0,
    user_rsp: u64 = 0,
    user_rflags: u64 = 0,

    kbase: u64 = sym.default_kbase,
    kheap: u64 = 0,
    kaslr: bool = true,

    pub fn kbase_offset(self: *const ctx) u64 {
        return self.kbase -% sym.default_kbase;
    }

    // Translate an address based at default_kbase to the runtime address.
    pub fn kaddr(self: *const ctx, addr_at_default_kbase: u64) u64 {
        if (addr_at_default_kbase == sym.dummy_value) {
            return sym.dummy_value;
        }
        return addr_at_default_kbase +% self.kbase_offset();
    }

    pub fn is_valid_kbase(self: *const ctx) bool {
        if (!self.kaslr and self.kbase == sym.default_kbase) {
            return true;
        }

        if (self.kaslr and
            ((self.kbase >> 32) == 0xffffffff) and
            ((self.kbase & 0xfffff) == 0) and
            (sym.default_kbase <= self.kbase) and
            (self.kbase <= 0xffffffffc0000000))
        {
            return true;
        }

        return false;
    }
};
