#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "function.h"

#define SYS_my_get_physical_address 452

void *my_get_physical_addresses(void *virtual_address)
{
	return (void *)syscall(SYS_my_get_physical_address, virtual_address);
}
