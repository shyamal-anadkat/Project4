#ifndef CONDVAR_H
#define CONDVAR_H

#include "queue.h"
#include "mutex.h"

//stuct for condvar
struct condvar {
	struct Queue queue;
	struct mutex mtx;
};

void cv_init(struct condvar* cv);
void cv_wait(struct condvar* cv, struct mutex* mtx);
void cv_signal(struct condvar* cv);
void cv_broadcast(struct condvar* cv);

#endif
