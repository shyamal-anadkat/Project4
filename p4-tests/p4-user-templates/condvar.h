
#ifndef CONDVAR_H
#define CONDVAR_H

struct condvar {
	/* fill this in! */
};

void cv_init(struct condvar* cv);
void cv_wait(struct condvar* cv, struct mutex* mtx);
void cv_signal(struct condvar* cv);
void cv_broadcast(struct condvar* cv);

#endif
