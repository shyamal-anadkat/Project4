#include "types.h"
#include "user.h"

void cv_init(struct condvar* cv)
{
	/* fill this in! */
	cv->queue = initQueue(61);
	mutex_init(&cv->mtx);
}

void cv_wait(struct condvar* cv, struct mutex* mtx)
{
	/* fill this in! */
	// lock cv
	mutex_lock(&cv->mtx);
	//add to queue
	//setpark
	//unlock mtx
	//unlock cv mtx
	//park
	//lock mtx

}

void cv_signal(struct condvar* cv)
{
	/* fill this in! */
}

void cv_broadcast(struct condvar* cv)
{
	/* fill this in! */
}
