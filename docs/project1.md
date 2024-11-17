# Project 1：實作回傳 Physical Address 的 System Call

第 17 組：<br>
朱巽葦（）、陳禾旻（109502562）、洪柏軒（）、林睿瀚（）

:::info
[Github Repo](https://github.com/hermeschen1116/linux-systemcall-implement)
:::

[ToC]

## 環境

- Debian GNU/Linux 12 (bookworm) x86_64
- Linux Kernel 6.1.0
- QEMU/KVM or VMware

## 實作

### 編譯 & 安裝 Kernel

### 新增 System Call

### ```my_get_physical_addresses```

- 程式碼

```clike=
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
	if (pgd_none(*pgd)) {
		printk(KERN_WARNING "my_get_physical_addresses: Invalid PGD\n");
		return 0;
	}

	p4d = p4d_offset(pgd, virtual_address);
	if (p4d_none(*p4d)) {
		printk(KERN_WARNING "my_get_physical_addresses: Invalid P$D\n");
		return 0;
	}

	pud = pud_offset(p4d, virtual_address);
	if (pud_none(*pud)) {
		printk(KERN_WARNING "my_get_physical_addresses: Invalid PUD\n");
		return 0;
	}

	pmd = pmd_offset(pud, virtual_address);
	if (pmd_none(*pmd)) {
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
	printk(KERN_DEBUG
	       "my_get_physical_addresses:\nvirtual address: %lx\npage frame number: %lx, page offset: %lx\nphysical address: %lx\n",
	       virtual_address, (page_frame_number >> PAGE_SHIFT), page_offset,
	       physical_address);

	return physical_address;
}
```

#### 測試

##### Q1

- 程式碼

```clike=
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define SYS_my_get_physical_address 452

unsigned long my_get_physical_addresses(void *virtual_address);

int main()
{
	int *shared_memory = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
				  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (shared_memory == MAP_FAILED) {
		perror("mmap failed");
		exit(1);
	}
	*shared_memory = 48763;

	printf("Initial value in parent process: %d\n", *shared_memory);
	printf("Parent's physical address before fork: [0x%lx]\n",
	       my_get_physical_addresses(shared_memory));
	unsigned long parent_physical_address_before =
		my_get_physical_addresses(shared_memory);

	pid_t pid = fork();
	if (pid < 0) {
		perror("fork failed");
		exit(1);
	} else if (pid == 0) {
		printf("Child's physical address before write: [0x%lx]\n",
		       my_get_physical_addresses(shared_memory));

		*shared_memory = 114514;
		printf("New value in child process: %d\n", *shared_memory);

		printf("Child's physical address after write: [0x%lx]\n",
		       my_get_physical_addresses(shared_memory));

		exit(0);
	} else {
		wait(NULL);

		printf("Parent's physical address after child's write: [0x%lx]\n",
		       my_get_physical_addresses(shared_memory));
		unsigned long parent_physical_address_after =
			my_get_physical_addresses(shared_memory);

		if (parent_physical_address_after ==
		    parent_physical_address_before) {
			printf("Parent's physical address remains the same.\n");
		} else {
			printf("Parent's physical address has changed (unexpected in COW).\n");
		}
	}

	munmap(shared_memory, 4096);
	return 0;
}

unsigned long my_get_physical_addresses(void *virtual_address)
{
	return syscall(SYS_my_get_physical_address, virtual_address);
}
```

- 結果

##### Q2

- 程式碼

```clike=
#include <stdio.h>
#include <unistd.h>

#define SYS_my_get_physical_address 452

unsigned long my_get_physical_addresses(void *virtual_address);

int a[2000000];

int main()
{
	int loc_a;

	printf("global element a[0]:\n");
	printf("Offest of logical address:[%p]   Physical address:[0x%lx]\n",
	       &a[0], my_get_physical_addresses(&a[0]));
	printf("========================================================================\n");
	printf("global element a[1999999]:\n");
	printf("Offest of logical address:[%p]   Physical address:[0x%lx]\n",
	       &a[1999999], my_get_physical_addresses(&a[1999999]));
	printf("========================================================================\n");
}

unsigned long my_get_physical_addresses(void *virtual_address)
{
	return syscall(SYS_my_get_physical_address, virtual_address);
}
```

- 結果

## 參考資料

- [Add a System call to the Linux Kernel (6.0.9) in Ubuntu 22.04 using Oracle VM VirtualBox at Window](https://medium.com/@rajiv.cse/add-a-system-call-to-the-linux-kernel-6-0-9-in-ubuntu-22-04-acd7f7afc933)
- [Linux 核心 Copy On Write 實作機制](https://hackmd.io/@linD026/Linux-kernel-COW-Copy-on-Write#Physical-Memory-Model)