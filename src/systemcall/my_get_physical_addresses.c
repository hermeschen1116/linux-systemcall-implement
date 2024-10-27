#include <asm/page.h>
#include <asm/pgtable.h>
#include <linux/highmem.h>
#include <linux/mm.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>

SYSCALL_DEFINE1(my_get_physical_addresses, unsigned long, user_addr) {
  struct mm_struct *mm = current->mm;
  pgd_t *pgd;
  p4d_t *p4d;
  pud_t *pud;
  pmd_t *pmd;
  pte_t *pte;
  unsigned long physical_addr = 0;
  unsigned long vaddr = user_addr;

  // Check if the virtual address is valid
  if (!access_ok((void __user *)vaddr, sizeof(unsigned long))) {
    return 0; // Invalid address
  }

  // Walk the page table
  pgd = pgd_offset(mm, vaddr);
  if (pgd_none(*pgd) || pgd_bad(*pgd))
    return 0;

  p4d = p4d_offset(pgd, vaddr);
  if (p4d_none(*p4d) || p4d_bad(*p4d))
    return 0;

  pud = pud_offset(p4d, vaddr);
  if (pud_none(*pud) || pud_bad(*pud))
    return 0;

  pmd = pmd_offset(pud, vaddr);
  if (pmd_none(*pmd) || pmd_bad(*pmd))
    return 0;

  // Find the page table entry
  pte = pte_offset_map(pmd, vaddr);
  if (!pte || pte_none(*pte))
    return 0; // No physical address assigned

  if (pte_present(*pte)) {
    // Get the page frame number (PFN) and calculate the physical address
    physical_addr = (pte_pfn(*pte) << PAGE_SHIFT) | (vaddr & ~PAGE_MASK);
  }

  pte_unmap(pte);

  return physical_addr;
}
