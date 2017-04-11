#include "types.h"
#include "user.h"

void sem_init(struct semaphore* sem, int initval)
{
	/* fill this in! */
	mutex_init(&sem->mtx);            //lock init
	sem->counter = initval; 		  //init counter to initVal
	cv_init(&sem->cond);			  //init cv
}

//decrement the value of semaphore s by one
//wait if value of semaphore s is negative

void sem_wait(struct semaphore* sem)
{
	
	mutex_lock(&sem->mtx);

	while(sem->counter == 0) {
		cv_wait(&sem->cond, &sem->mtx);
	}
	sem->counter--;

	mutex_unlock(&sem->mtx);
}

//increment the value of semaphore s by one
//if there are one or more threads waiting, wake one

void sem_post(struct semaphore* sem)
{
		mutex_lock(&sem->mtx);

		//increment sem value
		sem->counter++;    

		//wake one up 
		cv_broadcast(&sem->cond);

	    mutex_unlock(&sem->mtx);
}