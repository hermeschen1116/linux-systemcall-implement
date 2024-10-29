#include <linux/syscalls.h>
// #define DEBUG

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
#ifdef DEBUG
	printk("pgd_val = 0x%lx\n", pgd_val(*pgd));
	printk("pgd_index = %lu\n", pgd_index(vaddr));
#endif
	if (pgd_none(*pgd)) {
		printk("not mapped in pgd\n");
		return 0;
	}

	p4d = p4d_offset(pgd, vaddr);
#ifdef DEBUG
	printk("p4d_val = 0x%lx\n", p4d_val(*p4d));
	printk("p4d_index = %lu\n", p4d_index(vaddr));
#endif
	if (p4d_none(*p4d)) {
		printk("not mapped in p4d\n");
		return 0;
	}

	pud = pud_offset(p4d, vaddr);
#ifdef DEBUG
	printk("pud_val = 0x%lx\n", pud_val(*pud));
	printk("pud_index = %lu\n", pud_index(vaddr));
#endif
	if (pud_none(*pud)) {
		printk("not mapped in pud\n");
		return 0;
	}

	pmd = pmd_offset(pud, vaddr);
#ifdef DEBUG
	printk("pmd_val = 0x%lx\n", pmd_val(*pmd));
	printk("pmd_index = %lu\n", pmd_index(vaddr));
#endif
	if (pmd_none(*pmd)) {
		printk("not mapped in pmd\n");
		return 0;
	}

	pte = pte_offset_kernel(pmd, vaddr);
#ifdef DEBUG
	printk("pte_val = 0x%lx\n", pte_val(*pte));
	printk("pte_index = %lu\n", pte_index(vaddr));
#endif
	if (pte_none(*pte)) {
		printk("not mapped in pte\n");
		return 0;
	}

	/* Page frame physical address mechanism | offset */
	page_addr = pte_val(*pte) & PAGE_MASK;
	page_offset = vaddr & ~PAGE_MASK;
	paddr = page_addr | page_offset;
#ifdef DEBUG
	printk("page_addr = %lx, page_offset = %lx\n", page_addr, page_offset);
	printk("vaddr = %lx, paddr = %lx\n", vaddr, paddr);
#endif

	return paddr;
}
