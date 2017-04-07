
#ifndef SEMAPHORE_H
#define SEMAPHORE_H

struct semaphore {
	/* fill this in! */
};

void sem_init(struct semaphore* sem, int initval);
void sem_post(struct semaphore* sem);
void sem_wait(struct semaphore* sem);

#endif
