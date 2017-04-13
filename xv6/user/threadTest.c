#include "types.h"
#include "stat.h"
#include "user.h"
#include "threads.h"

void test(void *arg)
{
	int i;
	for (i = 0; i < 299999999; i++)
		;

	int x = 5;
	printf(1, "x at %p\n", &x);
	printf(1, "thread with pid: %d\n", getpid());
	exit();
}

void snooze(void *argSnooze)
{
	printf(1, "ZzzzZzzzZzzz .... %d\n", getpid());
	setpark();
	park();
	printf(1, "I'm awake, I'm awake!\n");
	exit();
}

void alarm(void *argAlarm)
{
	int i;
	printf(1, "Spinning.... %d\n", getpid());
	for (i = 0; i < 9999999; i++)
		;
	unpark(4);
	for (i = 0; i < 9999999; i++)
		;
	printf(1, "Done spinning.... %d\n", getpid());
	exit();
}

int main() 
{
	int x = 10;
	printf(1, "Main x at %p\n", &x);
	int pid = thread_create(snooze, (void*)x);
	int pid2 = thread_create(alarm, (void*)x);
	thread_join();
	thread_join();
	printf(1, "Thread1 exited w/ pid %d\n", pid);
	printf(1, "Thread2 exited w/ pid %d\n", pid2);

	exit();
}

