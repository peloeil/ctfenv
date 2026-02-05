#include <stdint.h>

inline uint64_t align_to_page(const uint64_t addr) {
    return addr & ~0xfff;
}

inline uint64_t vaddr_to_pgd_index(const uint64_t vaddr) {
    return (vaddr >> 39) & 0x1ff;
}

inline uint64_t vaddr_to_pud_index(const uint64_t vaddr) {
    return (vaddr >> 30) & 0x1ff;
}

inline uint64_t vaddr_to_pmd_index(const uint64_t vaddr) {
    return (vaddr >> 21) & 0x1ff;
}

inline uint64_t vaddr_to_pte_index(const uint64_t vaddr) {
    return (vaddr >> 12) & 0x1ff;
}

inline uint64_t vaddr_to_page_offset(const uint64_t vaddr) {
    return vaddr & 0xfff;
}

inline uint64_t pgd_index_to_vpart(const uint64_t pgd_index) {
    return pgd_index << 39;
}

inline uint64_t pud_index_to_vpart(const uint64_t pud_index) {
    return pud_index << 30;
}

inline uint64_t pmd_index_to_vpart(const uint64_t pmd_index) {
    return pmd_index << 21;
}

inline uint64_t pte_index_to_vpart(const uint64_t pte_index) {
    return pte_index << 12;
}

inline uint64_t phys_page_to_pte(const uint64_t paddr) {
    return paddr | 0x8000000000000067;
}

inline uint64_t pte_to_phys_page(const uint64_t pte) {
    return pte & 0xffffffffff000;
}

inline uint64_t pt_index_to_vaddr(const uint64_t pgd_index, const uint64_t pud_index,
                                  const uint64_t pmd_index, const uint64_t pte_index,
                                  const uint64_t page_offset) {
    return pgd_index_to_vpart(pgd_index) | pud_index_to_vpart(pud_index) |
           pmd_index_to_vpart(pmd_index) | pte_index_to_vpart(pte_index) | page_offset;
}
