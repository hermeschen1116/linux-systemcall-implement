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
