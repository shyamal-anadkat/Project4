
#ifndef MUTEX_H
#define MUTEX_H

struct mutex {
	// TODO remove comment
	//pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
	int flag;
	int guard;
    queue_t *q;
};

void mutex_init(struct mutex* mtx);
void mutex_lock(struct mutex* mtx);
void mutex_unlock(struct mutex* mtx);

#endif