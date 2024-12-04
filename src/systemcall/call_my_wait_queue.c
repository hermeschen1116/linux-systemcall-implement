#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/slab.h>

static wait_queue_head_t my_wait_queue;
static int queue_initialized = 0;

static void initialize_wait_queue(void)
{
	if (!queue_initialized) {
		init_waitqueue_head(&my_wait_queue);
		queue_initialized = 1;
	}
}

static int enter_wait_queue(void)
{
	DEFINE_WAIT(wait);

	initialize_wait_queue();

	add_wait_queue(&my_wait_queue, &wait);
	prepare_to_wait(&my_wait_queue, &wait, TASK_INTERRUPTIBLE);

	schedule();

	if (signal_pending(current)) {
		finish_wait(&my_wait_queue, &wait);
		return 0;
	}

	finish_wait(&my_wait_queue, &wait);
	return 1;
}

static int clean_wait_queue(void)
{
	initialize_wait_queue();

	if (waitqueue_active(&my_wait_queue)) {
		wake_up(&my_wait_queue);
		return 1;
	}
	return 0;
}

SYSCALL_DEFINE1(call_my_wait_queue, int, id)
{
	switch (id) {
	case 1:
		return enter_wait_queue();
	case 2:
		return clean_wait_queue();
	default:
		return 0;
	}
}
