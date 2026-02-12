// User-editable kernel symbols/gadgets.
// Values are assumed to be addresses from vmlinux (i.e. based at default_kbase).

pub const dummy_value: u64 = 0xdddddddddddddddd;
pub const default_kbase: u64 = 0xffffffff81000000;

pub const page_size: u64 = 0x1000;

// ------------------- uneditable -------------------
pub const config_physical_start: u64 = 0x1000000;
pub const config_physical_align: u64 = 0x200000;
pub const config_randomize_base_max_offset: u64 = 0x40000000;
pub const pud_size: u64 = 0x40000000;

// -------------------- editable --------------------
// addresses
pub const addr_prepare_kernel_cred: u64 = dummy_value;
pub const addr_init_cred: u64 = dummy_value;
pub const addr_commit_creds: u64 = dummy_value;
pub const addr_modprobe_path: u64 = dummy_value;

// gadgets
pub const pop_rdi_ret: u64 = dummy_value;
pub const pop_rcx_ret: u64 = dummy_value;
pub const mov_rdi_rax_rep_ret: u64 = dummy_value;
pub const bypass_kpti: u64 = dummy_value;
pub const mov_prdx_rcx_ret: u64 = dummy_value;
pub const mov_rax_prdx_ret: u64 = dummy_value;
pub const mov_esp_0x39000000_ret: u64 = dummy_value;
pub const add_rsp_0x140_pop6_ret: u64 = dummy_value;
