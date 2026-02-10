#ifndef __KPWN_PAGE_H
#define __KPWN_PAGE_H

#include <stdint.h>

uint64_t align_to_page(const uint64_t addr);
uint64_t vaddr_to_pgd_index(const uint64_t vaddr);
uint64_t vaddr_to_pud_index(const uint64_t vaddr);
uint64_t vaddr_to_pmd_index(const uint64_t vaddr);
uint64_t vaddr_to_pte_index(const uint64_t vaddr);
uint64_t vaddr_to_page_offset(const uint64_t vaddr);
uint64_t pgd_index_to_vpart(const uint64_t pgd_index);
uint64_t pud_index_to_vpart(const uint64_t pud_index);
uint64_t pmd_index_to_vpart(const uint64_t pmd_index);
uint64_t pte_index_to_vpart(const uint64_t pte_index);
uint64_t phys_page_to_pte(const uint64_t paddr);
uint64_t pte_to_phys_page(const uint64_t pte);
uint64_t pt_index_to_vaddr(const uint64_t pgd_index, const uint64_t pud_index,
                           const uint64_t pmd_index, const uint64_t pte_index,
                           const uint64_t page_offset);

#endif  // __KPWN_PAGE_H
