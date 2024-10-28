#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/highmem.h>

SYSCALL_DEFINE1(my_get_physical_addresses, void *, virtual_address)
{
	unsigned long vaddr;
	struct page *page;
	unsigned long paddr = 0;
	void *ret_addr = NULL;

	// Step 1: Copy the virtual address from user space to kernel space
	if (copy_from_user(&vaddr, &virtual_address, sizeof(void *))) {
		return (void *)0; // Return 0 if address copy fails
	}

	// Step 2: Get the struct page associated with the virtual address
	page = follow_page(current->mm, (void *)vaddr, FOLL_GET);
	if (!page) {
		return (void *)0; // Return 0 if no physical page is mapped
	}

	// Step 3: Calculate the physical address from the page structure
	paddr = page_to_pfn(page) << PAGE_SHIFT;
	paddr |= (vaddr & ~PAGE_MASK); // add offset within the page

	// Step 4: Copy the result back to user space
	ret_addr = (void *)paddr;
	if (copy_to_user(virtual_address, &ret_addr, sizeof(void *))) {
		return (void *)0; // Return 0 if address copy fails
	}

	// Step 5: Return the physical address to user space
	return ret_addr;
}
