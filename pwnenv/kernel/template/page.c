#include <stdint.h>

__always_inline uint64_t addr_to_page(const uint64_t addr) {
    return addr & ~0xfff;
}

__always_inline uint64_t vaddr_to_phys_index(const uint64_t vaddr) {
    return vaddr & 0xfff;
}

__always_inline uint64_t vaddr_to_pgd_index(const uint64_t vaddr) {
    return (vaddr >> 39) & 0x1ff;
}

__always_inline uint64_t vaddr_to_pud_index(const uint64_t vaddr) {
    return (vaddr >> 30) & 0x1ff;
}

__always_inline uint64_t vaddr_to_pmd_index(const uint64_t vaddr) {
    return (vaddr >> 21) & 0x1ff;
}

__always_inline uint64_t vaddr_to_pte_index(const uint64_t vaddr) {
    return (vaddr >> 12) & 0x1ff;
}

__always_inline uint64_t vaddr_to_page_offset(const uint64_t vaddr) {
    return vaddr & 0xfff;
}

__always_inline uint64_t paddr_to_pte(const uint64_t paddr) {
    return paddr | 0x8000000000000067;
}

__always_inline uint64_t pte_to_paddr(const uint64_t pte) {
    return pte & 0xffffffffff000;
}
