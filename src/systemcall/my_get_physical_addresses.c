#include <linux/syscalls.h>

SYSCALL_DEFINE1(my_get_physical_addresses, void *, user_virtual_address)
{
	unsigned long virtual_address = (unsigned long)user_virtual_address;
	pgd_t *pgd;
	p4d_t *p4d;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;
	unsigned long page_frame_number = 0;
	unsigned long page_offset = 0;
	unsigned long physical_address = 0;

	// Walk through the tables
	pgd = pgd_offset(current->mm, virtual_address);
	if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd))) {
		printk(KERN_WARNING "my_get_physical_addresses: Invalid PGD\n");
		return 0;
	}

	p4d = p4d_offset(pgd, virtual_address);
	if (p4d_none(*p4d) || unlikely(p4d_bad(*p4d))) {
		printk(KERN_WARNING "my_get_physical_addresses: Invalid P$D\n");
		return 0;
	}

	pud = pud_offset(p4d, virtual_address);
	if (pud_none(*pud) || unlikely(pud_bad(*pud))) {
		printk(KERN_WARNING "my_get_physical_addresses: Invalid PUD\n");
		return 0;
	}

	pmd = pmd_offset(pud, virtual_address);
	if (pmd_none(*pmd) || unlikely(pmd_bad(*pmd))) {
		printk(KERN_WARNING "my_get_physical_addresses: Invalid PMD\n");
		return 0;
	}

	// Find the page table entry
	pte = pte_offset_kernel(pmd, virtual_address);
	if (pte_none(*pte)) {
		printk(KERN_WARNING
		       "my_get_physical_addresses: No PTE found\n");
		return 0;
	}

	// calculate physical address
	page_frame_number = pte_val(*pte) & PAGE_MASK;
	page_offset = virtual_address & ~PAGE_MASK;
	physical_address = page_frame_number | page_offset;
	printk(KERN_DEBUG "my_get_physical_addresses:\nvirtual address: %lx\n",
	       virtual_address);
	printk(KERN_DEBUG
	       "my_get_physical_addresses:\npage frame number: %lx, page offset: %lx\n",
	       (page_frame_number >> PAGE_SHIFT), page_offset);
	printk(KERN_DEBUG "my_get_physical_addresses:\nphysical address: %lx\n",
	       physical_address);

	return physical_address;
}
