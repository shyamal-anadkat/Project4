
#ifndef MUTEX_H
#define MUTEX_H

struct mutex {
	/* fill this in! */
};

void mutex_init(struct mutex* mtx);
void mutex_lock(struct mutex* mtx);
void mutex_unlock(struct mutex* mtx);


#endif
