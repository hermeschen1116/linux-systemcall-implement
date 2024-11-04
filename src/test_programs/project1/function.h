#ifndef FUNCTION_H
#define FUNCTION_H

#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

void *my_get_physical_addresses(void *virtual_address);

#endif
