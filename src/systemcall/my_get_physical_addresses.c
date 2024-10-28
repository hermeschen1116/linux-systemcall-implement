#include <asm/page.h>
#include <asm/pgtable.h>
#include <linux/highmem.h>
#include <linux/mm.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>

SYSCALL_DEFINE1(my_get_physical_addresses, void *, user_virtual_address)
{
	struct mm_struct *mm;
	unsigned long virtual_address;
	pgd_t *pgd;
	p4d_t *p4d;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;
	unsigned long physical_address;

	// Copy the virtual address from user space
	if (copy_from_user(&virtual_address, user_virtual_address,
			   sizeof(void *))) {
		return 0;
	}

	// Get the current process's memory map
	mm = current->mm;

	// Walk the page table
	pgd = pgd_offset(mm, virtual_address);
	if (pgd_none(*pgd) || pgd_bad(*pgd)) {
		return 0;
	}

	p4d = p4d_offset(pgd, virtual_address);
	if (p4d_none(*p4d) || p4d_bad(*p4d)) {
		return 0;
	}

	pud = pud_offset(p4d, virtual_address);
	if (pud_none(*pud) || pud_bad(*pud)) {
		return 0;
	}

	pmd = pmd_offset(pud, virtual_address);
	if (pmd_none(*pmd) || pmd_bad(*pmd)) {
		return 0;
	}

	// Find the page table entry
	pte = pte_offset_map(pmd, virtual_address);
	if (!pte_present(*pte)) {
		pte_unmap(pte);
		return 0; // No physical address assigned
	}

	// Get the page frame number (PFN) and calculate the physical address
	physical_address = (pte_pfn(*pte) << PAGE_SHIFT) |
			   (virtual_address & ~PAGE_MASK);

	pte_unmap(pte);

	return physical_address;
}
