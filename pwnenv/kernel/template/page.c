#include <stdint.h>

u64 align_to_page(const u64 addr) {
    return addr & ~0xfff;
}

u64 vaddr_to_pgd_index(const u64 vaddr) {
    return (vaddr >> 39) & 0x1ff;
}

u64 vaddr_to_pud_index(const u64 vaddr) {
    return (vaddr >> 30) & 0x1ff;
}

u64 vaddr_to_pmd_index(const u64 vaddr) {
    return (vaddr >> 21) & 0x1ff;
}

u64 vaddr_to_pte_index(const u64 vaddr) {
    return (vaddr >> 12) & 0x1ff;
}

u64 vaddr_to_page_offset(const u64 vaddr) {
    return vaddr & 0xfff;
}

u64 pgd_index_to_vpart(const u64 pgd_index) {
    return pgd_index << 39;
}

u64 pud_index_to_vpart(const u64 pud_index) {
    return pud_index << 30;
}

u64 pmd_index_to_vpart(const u64 pmd_index) {
    return pmd_index << 21;
}

u64 pte_index_to_vpart(const u64 pte_index) {
    return pte_index << 12;
}

u64 phys_page_to_pte(const u64 paddr) {
    return paddr | 0x8000000000000067;
}

u64 pte_to_phys_page(const u64 pte) {
    return pte & 0xffffffffff000;
}

u64 pt_index_to_vaddr(const u64 pgd_index, const u64 pud_index,
                           const u64 pmd_index, const u64 pte_index,
                           const u64 page_offset) {
    return pgd_index_to_vpart(pgd_index) | pud_index_to_vpart(pud_index) |
           pmd_index_to_vpart(pmd_index) | pte_index_to_vpart(pte_index) | page_offset;
}
