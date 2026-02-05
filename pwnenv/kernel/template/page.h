#ifndef __KPWN_PAGE_H
#define __KPWN_PAGE_H

#include <stdint.h>

inline uint64_t align_to_page(const uint64_t addr);
inline uint64_t vaddr_to_pgd_index(const uint64_t vaddr);
inline uint64_t vaddr_to_pud_index(const uint64_t vaddr);
inline uint64_t vaddr_to_pmd_index(const uint64_t vaddr);
inline uint64_t vaddr_to_pte_index(const uint64_t vaddr);
inline uint64_t vaddr_to_page_offset(const uint64_t vaddr);
inline uint64_t pgd_index_to_vpart(const uint64_t pgd_index);
inline uint64_t pud_index_to_vpart(const uint64_t pud_index);
inline uint64_t pmd_index_to_vpart(const uint64_t pmd_index);
inline uint64_t pte_index_to_vpart(const uint64_t pte_index);
inline uint64_t phys_page_to_pte(const uint64_t paddr);
inline uint64_t pte_to_phys_page(const uint64_t pte);
inline uint64_t pt_index_to_vaddr(const uint64_t pgd_index, const uint64_t pud_index,
                                  const uint64_t pmd_index, const uint64_t pte_index,
                                  const uint64_t page_offset);

#endif  // __KPWN_PAGE_H
