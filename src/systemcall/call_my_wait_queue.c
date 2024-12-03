#include <linux/syscalls.h>

static int enter_wait_queue(void)
{
	return 0;
}

static int clean_wait_queue(void)
{
	return 0;
}

SYSCALL_DEFINE1(call_wait_queue, int, id)
{
	switch (id) {
	case 1:
		enter_wait_queue() break;
	case 2:
		clean_wait_queue();
		break;
	}
	return 0;
}
