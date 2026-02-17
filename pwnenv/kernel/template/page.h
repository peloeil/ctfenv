#ifndef __KPWN_PAGE_H
#define __KPWN_PAGE_H

#include "types.h"

u64 align_to_page(const u64 addr);
u64 vaddr_to_pgd_index(const u64 vaddr);
u64 vaddr_to_pud_index(const u64 vaddr);
u64 vaddr_to_pmd_index(const u64 vaddr);
u64 vaddr_to_pte_index(const u64 vaddr);
u64 vaddr_to_page_offset(const u64 vaddr);
u64 pgd_index_to_vpart(const u64 pgd_index);
u64 pud_index_to_vpart(const u64 pud_index);
u64 pmd_index_to_vpart(const u64 pmd_index);
u64 pte_index_to_vpart(const u64 pte_index);
u64 phys_page_to_pte(const u64 paddr);
u64 pte_to_phys_page(const u64 pte);
u64 pt_index_to_vaddr(const u64 pgd_index, const u64 pud_index, const u64 pmd_index,
                      const u64 pte_index, const u64 page_offset);

#endif  // __KPWN_PAGE_H
