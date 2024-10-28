#include <syscall.h>
#include <sys/types.h>

int main(void)
{
	int a = syscall(451);
	return 0;
}
