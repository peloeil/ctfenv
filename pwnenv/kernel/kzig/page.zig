pub fn align_to_page(addr: u64) u64 {
    return addr & ~@as(u64, 0xfff);
}

pub fn vaddr_to_pgd_index(vaddr: u64) u64 {
    return (vaddr >> 39) & 0x1ff;
}

pub fn vaddr_to_pud_index(vaddr: u64) u64 {
    return (vaddr >> 30) & 0x1ff;
}

pub fn vaddr_to_pmd_index(vaddr: u64) u64 {
    return (vaddr >> 21) & 0x1ff;
}

pub fn vaddr_to_pte_index(vaddr: u64) u64 {
    return (vaddr >> 12) & 0x1ff;
}

pub fn vaddr_to_page_offset(vaddr: u64) u64 {
    return vaddr & 0xfff;
}

pub fn pgd_index_to_vpart(pgd_index: u64) u64 {
    return pgd_index << 39;
}

pub fn pud_index_to_vpart(pud_index: u64) u64 {
    return pud_index << 30;
}

pub fn pmd_index_to_vpart(pmd_index: u64) u64 {
    return pmd_index << 21;
}

pub fn pte_index_to_vpart(pte_index: u64) u64 {
    return pte_index << 12;
}

pub fn phys_page_to_pte(paddr: u64) u64 {
    return paddr | 0x8000000000000067;
}

pub fn pte_to_phys_page(pte: u64) u64 {
    return pte & 0xffffffffff000;
}

pub fn pt_index_to_vaddr(pgd_index: u64, pud_index: u64, pmd_index: u64, pte_index: u64, page_offset: u64) u64 {
    return pgd_index_to_vpart(pgd_index) |
        pud_index_to_vpart(pud_index) |
        pmd_index_to_vpart(pmd_index) |
        pte_index_to_vpart(pte_index) |
        page_offset;
}
