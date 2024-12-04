#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>

#define NUM_THREADS 10
#define SYS_call_my_wait_queue 453

void *enter_wait_queue(void *thread_id)
{
	fprintf(stderr, "enter wait queue thread_id: %d\n", *(int *)thread_id);
	long result = syscall(SYS_call_my_wait_queue, 1);

	if (result != 1) {
		fprintf(stderr,
			"Error: syscall failed with result %ld for thread_id: %d\n",
			result, *(int *)thread_id);
		free(thread_id);
		pthread_exit(NULL);
	}

	fprintf(stderr, "exit wait queue thread_id: %d\n", *(int *)thread_id);
	free(thread_id);
	return NULL;
}

void clean_wait_queue()
{
	long result = syscall(SYS_call_my_wait_queue, 2);
	if (result != 1) {
		fprintf(stderr,
			"Error: Failed to clean wait queue. Return value: %ld\n",
			result);
	}
}

int main()
{
	void *ret;
	pthread_t id[NUM_THREADS];
	int thread_args[NUM_THREADS];
	for (int i = 0; i < NUM_THREADS; i++) {
		thread_args[i] = i;
		pthread_create(&id[i], NULL, enter_wait_queue,
			       (void *)&thread_args[i]);
	}
	sleep(1);
	fprintf(stderr, "start clean queue ...\n");
	clean_wait_queue();
	for (int i = 0; i < NUM_THREADS; i++) {
		pthread_join(id[i], &ret);
	}
	return 0;
}
