#include <linux/syscalls.h>
#include <linux/wait.h>
#include <linux/sched.h>

static wait_queue_head_t my_wait_queue;
static int queue_initialized = 0;

static void initialize_wait_queue(void)
{
	if (!queue_initialized) {
		init_waitqueue_head(&my_wait_queue);
		queue_initialized = 1;
		printk(KERN_DEBUG
		       "call_my_wait_queue: my_wait_queue initialized.\n");
	}
}

static int enter_wait_queue(void)
{
	DEFINE_WAIT(wait);

	initialize_wait_queue();

	printk(KERN_DEBUG
	       "call_my_wait_queue: Process [%d] entering wait queue.\n",
	       current->pid);

	add_wait_queue(&my_wait_queue, &wait);
	prepare_to_wait(&my_wait_queue, &wait, TASK_INTERRUPTIBLE);
	schedule();

	if (signal_pending(current)) {
		printk(KERN_WARNING
		       "call_my_wait_queue: Process [%d] interrupted by signal.\n",
		       current->pid);
		finish_wait(&my_wait_queue, &wait);
		return 0;
	}

	finish_wait(&my_wait_queue, &wait);
	printk(KERN_DEBUG
	       "call_my_wait_queue: Process [%d] exited wait queue successfully.\n",
	       current->pid);
	return 1;
}

static int clean_wait_queue(void)
{
	initialize_wait_queue();

	if (!waitqueue_active(&my_wait_queue)) {
		printk(KERN_WARNING
		       "call_my_wait_queue: Attempt to clean an empty wait queue.\n");
		return 0;
	}

	printk(KERN_DEBUG
	       "call_my_wait_queue: Cleaning wait queue. Waking up all processes.\n");

	wake_up(&my_wait_queue);

	if (!waitqueue_active(&my_wait_queue)) {
		printk(KERN_DEBUG
		       "call_my_wait_queue: Wait queue cleaned successfully.\n");
		return 1;
	}

	printk(KERN_WARNING
	       "call_my_wait_queue: Wait queue still active after clean attempt.\n");
	return 0;
}

SYSCALL_DEFINE1(call_my_wait_queue, int, id)
{
	printk(KERN_DEBUG
	       "call_my_wait_queue: call_my_wait_queue invoked with id=%d.\n",
	       id);

	switch (id) {
	case 1:
		return enter_wait_queue();
	case 2:
		return clean_wait_queue();
	default:
		printk(KERN_ERR
		       "call_my_wait_queue: Invalid id (%d) passed to call_my_wait_queue.\n",
		       id);
		return 0;
	}
}
