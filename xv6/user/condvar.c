#include "types.h"
#include "user.h"

void cv_init(struct condvar* cv)
{
	cv->queue = *(initQueue(65));
	mutex_init(&cv->mtx);
}

void cv_wait(struct condvar* cv, struct mutex* mtx)
{
	// lock cv
	mutex_lock(&cv->mtx);
	//add to queue
	enqueue(&cv->queue, getpid());
	//setpark
	setpark();
	//unlock mtx
	mutex_unlock(mtx);
	//unlock cv mtx
	mutex_unlock(&cv->mtx);
	//park
	park();
	//lock mtx
	mutex_lock(mtx);
}

void cv_signal(struct condvar* cv)
{
	mutex_lock(&cv->mtx);
	//wakes up one waiting thread on signal
	if (!isEmpty(&cv->queue)) {
		unpark(dequeue(&cv->queue));
	}

	mutex_unlock(&cv->mtx);
}

void cv_broadcast(struct condvar* cv)
{
	mutex_lock(&cv->mtx);
	
	//wakes up all waiting thread
	while (!isEmpty(&cv->queue)) {
		//iterate through queue and keep dequeing
		unpark(dequeue(&cv->queue));
	}

	mutex_unlock(&cv->mtx);
}
