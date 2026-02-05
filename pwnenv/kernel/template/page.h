#ifndef __KPWN_PAGE_H
#define __KPWN_PAGE_H

#include <stdint.h>

__always_inline uint64_t addr_to_page(const uint64_t addr);
__always_inline uint64_t vaddr_to_phys_index(const uint64_t vaddr);
__always_inline uint64_t vaddr_to_pgd_index(const uint64_t vaddr);
__always_inline uint64_t vaddr_to_pud_index(const uint64_t vaddr);
__always_inline uint64_t vaddr_to_pmd_index(const uint64_t vaddr);
__always_inline uint64_t vaddr_to_pte_index(const uint64_t vaddr);
__always_inline uint64_t vaddr_to_page_offset(const uint64_t vaddr);
__always_inline uint64_t paddr_to_pte(const uint64_t paddr);
__always_inline uint64_t pte_to_paddr(const uint64_t pte);

#endif  // __KPWN_PAGE_H
