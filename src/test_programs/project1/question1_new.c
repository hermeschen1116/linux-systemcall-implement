#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/wait.h>

// 定義自訂的 system call 編號
#define SYS_my_get_physical_address 452

// 自訂 system call 的函數
unsigned long get_physical_address(void *virtual_address)
{
	return syscall(SYS_my_get_physical_address, virtual_address);
}

int main()
{
	// 1. 定義一個變數
	int shared_var = 42;
	printf("Initial value in parent process: %d\n", shared_var);

	// 2. 查詢父進程中變數的物理地址
	unsigned long parent_physical_address =
		get_physical_address(&shared_var);
	printf("Parent's physical address before fork: 0x%lx\n",
	       parent_physical_address);

	// 3. 建立子進程
	pid_t pid = fork();
	if (pid < 0) {
		perror("fork failed");
		exit(1);
	} else if (pid == 0) {
		// 子進程

		// 查詢子進程中變數的物理地址（COW 尚未觸發）
		unsigned long child_physical_address =
			get_physical_address(&shared_var);
		printf("Child's physical address before write: 0x%lx\n",
		       child_physical_address);

		// 嘗試寫入，觸發 COW
		shared_var = 84;
		printf("New value in child process: %d\n", shared_var);

		// 查詢寫入後的物理地址
		child_physical_address = get_physical_address(&shared_var);
		printf("Child's physical address after write: 0x%lx\n",
		       child_physical_address);

		exit(0);
	} else {
		// 父進程：等待子進程完成
		wait(NULL);

		// 子進程寫入後，檢查父進程中的物理地址是否改變
		unsigned long new_parent_physical_address =
			get_physical_address(&shared_var);
		printf("Parent's physical address after child's write: 0x%lx\n",
		       new_parent_physical_address);

		// 4. 比較寫入前後的物理地址
		if (parent_physical_address == new_parent_physical_address) {
			printf("Parent's physical address remains the same.\n");
		} else {
			printf("Parent's physical address has changed (unexpected in COW).\n");
		}
	}

	return 0;
}
