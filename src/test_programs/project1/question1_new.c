#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdint.h>

// 自訂的 system call 編號 (假設是 451，實際需檢查)
#define SYS_my_get_physical_address 452

// 自訂 system call 的函數
unsigned long get_physical_address(void *virtual_address)
{
	return syscall(SYS_my_get_physical_address, virtual_address);
}

int main()
{
	// 1. 分配一個頁面大小的共享記憶體區段
	int *shared_memory = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
				  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (shared_memory == MAP_FAILED) {
		perror("mmap failed");
		exit(1);
	}

	// 2. 將頁面初始化為某個數值
	*shared_memory = 42;
	printf("Initial value in parent process: %d\n", *shared_memory);

	// 3. 查詢父進程中的虛擬地址對應的物理地址
	unsigned long parent_physical_address =
		get_physical_address(shared_memory);
	printf("Parent's physical address before fork: 0x%lx\n",
	       parent_physical_address);

	// 4. 建立子進程
	pid_t pid = fork();
	if (pid < 0) {
		perror("fork failed");
		exit(1);
	} else if (pid == 0) {
		// 子進程

		// 查詢子進程中共享頁面的物理地址（COW 機制尚未觸發）
		unsigned long child_physical_address =
			get_physical_address(shared_memory);
		printf("Child's physical address before write: 0x%lx\n",
		       child_physical_address);

		// 觸發 COW：嘗試寫入該頁面
		*shared_memory = 84;
		printf("New value in child process: %d\n", *shared_memory);

		// 查詢寫入後的物理地址
		child_physical_address = get_physical_address(shared_memory);
		printf("Child's physical address after write: 0x%lx\n",
		       child_physical_address);

		exit(0);
	} else {
		// 父進程：等待子進程完成
		wait(NULL);

		// 子進程寫入後，檢查父進程中的物理地址是否改變
		unsigned long new_parent_physical_address =
			get_physical_address(shared_memory);
		printf("Parent's physical address after child's write: 0x%lx\n",
		       new_parent_physical_address);

		// 5. 比較寫入前後的物理地址
		if (parent_physical_address == new_parent_physical_address) {
			printf("Parent's physical address remains the same.\n");
		} else {
			printf("Parent's physical address has changed (unexpected in COW).\n");
		}
	}

	// 6. 解除映射
	munmap(shared_memory, 4096);
	return 0;
}
