#include "types.h"
#include "user.h"
#include "x86.h"
#include "mutex.h"
#include "queue.h"

void mutex_init(struct mutex* mtx)
{
	mtx->flag  = 0;
	mtx->guard = 0;
    mtx->queue = initQueue(61);
    //queue_init(mtx->q);
}

// TODO remove comments
void mutex_lock(struct mutex* mtx)
{
    // acquire guard lock by spinning
	while(xchg((volatile uint*)&mtx->guard, 1) != 0);

    if (mtx->flag == 0) {
        mtx->flag = 1;
		mtx->guard = 0; 
	}
	else {
        //queue_add(mtx->q, gettid());
        enqueue(mtx->queue, getpid());
        setpark();
        mtx->guard = 0;
        park();
    }
}

void mutex_unlock(struct mutex* mtx)
{
	//acquire guard lock by spinning
    while (xchg((volatile uint*)&mtx->guard, 1) != 0);
    if (isEmpty(mtx->queue)) {
        mtx->flag = 0; // let go of lock; no one wants it
    }
    else {
        unpark(dequeue(mtx->queue)); // hold lock (for next thread!)
    }
    mtx->guard = 0;
}
