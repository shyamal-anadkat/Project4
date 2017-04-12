
#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "queue.h"
#include "mutex.h"
#include "condvar.h"

struct semaphore {
	struct condvar cond;
	struct mutex mtx; 
	int counter;
};

void sem_init(struct semaphore* sem, int initval);
void sem_post(struct semaphore* sem);
void sem_wait(struct semaphore* sem);

#endif
