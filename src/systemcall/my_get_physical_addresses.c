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

	// Walk the page table to find the page table entry
	pte = follow_pte(current->mm, virtual_address, NULL);
	if (!pte_present(*pte)) {
		printk(KERN_WARNING
		       "my_get_physical_addresses: No PTE found\n");
		return 0;
	}

	physical_address = (pte_pfn(*pte) << PAGE_SHIFT) |
			   (virtual_address & ~PAGE_MASK);

	pte_unmap(pte);

	return physical_address;
}
