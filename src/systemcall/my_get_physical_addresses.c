#include <linux/syscalls.h>

SYSCALL_DEFINE1(my_get_physical_addresses, void *, addr_p)
{
	unsigned long vaddr = (unsigned long)addr_p;
	pgd_t *pgd;
	p4d_t *p4d;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;
	unsigned long paddr = 0;
	unsigned long page_addr = 0;
	unsigned long page_offset = 0;

	pgd = pgd_offset(current->mm, vaddr);
	if (pgd_none(*pgd)) {
		printk("not mapped in pgd\n");
		return 0;
	}

	p4d = p4d_offset(pgd, vaddr);
	if (p4d_none(*p4d)) {
		printk("not mapped in p4d\n");
		return 0;
	}

	pud = pud_offset(p4d, vaddr);
	if (pud_none(*pud)) {
		printk("not mapped in pud\n");
		return 0;
	}

	pmd = pmd_offset(pud, vaddr);
	if (pmd_none(*pmd)) {
		printk("not mapped in pmd\n");
		return 0;
	}

	pte = pte_offset_kernel(pmd, vaddr);
	if (pte_none(*pte)) {
		printk("not mapped in pte\n");
		return 0;
	}

	/* Page frame physical address mechanism | offset */
	page_addr = pte_val(*pte) & PAGE_MASK;
	page_offset = vaddr & ~PAGE_MASK;
	paddr = page_addr | page_offset;

	return paddr;
}
