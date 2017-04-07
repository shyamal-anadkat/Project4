
#include "types.h"
#include "user.h"

void mutex_init(struct mutex* mtx)
{
	mtx->flag  = 0;
	mtx->guard = 0;
    queue_init(mtx->q);
}

// TODO remove comments
void mutex_lock(struct mutex* mtx)
{
	while(xchg(&mtx->guard, 1) != 0);
	//while (TestAndSet(&mtx->guard, 1) == 1) ; //acquire guard lock by spinning
    if (mtx->flag == 0) {
        mtx->flag = 1;
		mtx->guard = 0; 
	}
	else {
        queue_add(mtx->q, gettid());
        setpark();
        mtx->guard = 0;
        park();
    }
}

void mutex_unlock(struct mutex* mtx)
{
	while (xchg(&mtx->guard, 1) != 0);
	//while (TestAndSet(&mtx->guard, 1) == 1); //acquire guard lock by spinning
    if (queue_empty(mtx->q)) {
        mtx->flag = 0; // let go of lock; no one wants it
    }
    else {
        unpark(queue_remove(mtx->q)); // hold lock (for next thread!)
    }
    mtx->guard = 0;
}
