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
	printf("Parent's physical address before fork: [0x%p]\n",
	       (void *)my_get_physical_addresses(shared_memory));
	unsigned long parent_physical_address_before =
		my_get_physical_addresses(shared_memory);

	pid_t pid = fork();
	if (pid < 0) {
		perror("fork failed");
		exit(1);
	} else if (pid == 0) {
		printf("Child's physical address before write: [0x%p]\n",
		       (void *)my_get_physical_addresses(shared_memory));

		*shared_memory = 114514;
		printf("New value in child process: %d\n", *shared_memory);

		printf("Child's physical address after write: [0x%p]\n",
		       (void *)my_get_physical_addresses(shared_memory));

		exit(0);
	} else {
		wait(NULL);

		printf("Parent's physical address after child's write: [0x%p]\n",
		       (void *)my_get_physical_addresses(shared_memory));
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
