#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/mm.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <asm/pgtable.h>

SYSCALL_DEFINE1(my_get_physical_addresses, void *__user, user_virtual_address)
{
	pgd_t *pgd;
	p4d_t *p4d;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;
	unsigned long virtual_address;
	unsigned long physical_address = 0;

	// Copy the virtual address from user space
	if (!access_ok((void *__user)user_virtual_address, sizeof(void *))) {
		printk(KERN_ERR
		       "my_get_physical_addresses: Invalid virtual address\n");
		return 0; // Invalid address
	}

	virtual_address = (unsigned long)user_virtual_address;

	// Walk the page table
	pgd = pgd_offset(current->mm, virtual_address);
	if (pgd_none(*pgd) || pgd_bad(*pgd)) {
		printk(KERN_WARNING "my_get_physical_addresses: Invalid PGD\n");
		return 0;
	}

	p4d = p4d_offset(pgd, virtual_address);
	if (p4d_none(*p4d) || p4d_bad(*p4d)) {
		printk(KERN_WARNING "my_get_physical_addresses: Invalid P$D\n");
		return 0;
	}

	pud = pud_offset(p4d, virtual_address);
	if (pud_none(*pud) || pud_bad(*pud)) {
		printk(KERN_WARNING "my_get_physical_addresses: Invalid PUD\n");
		return 0;
	}

	pmd = pmd_offset(pud, virtual_address);
	if (pmd_none(*pmd) || pmd_bad(*pmd)) {
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

	physical_address = (pte_val(*pte) & PAGE_MASK) |
			   (virtual_address & ~PAGE_MASK);

	pte_unmap(pte);

	return physical_address;
}
