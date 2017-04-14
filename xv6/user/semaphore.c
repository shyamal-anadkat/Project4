#include "types.h"
#include "user.h"

//****Implement semaphore using condition variables*****//

void sem_init(struct semaphore* sem, int initval)
{
	mutex_init(&sem->mtx);            //lock init
	sem->counter = initval; 		  //init counter to initVal
	cv_init(&sem->cond);			  //init cv
}

//decrement the value of semaphore s by one
//wait if value of semaphore s is negative

void sem_wait(struct semaphore* sem)
{
	mutex_lock(&sem->mtx);	//make atomic

	while(sem->counter == 0) {
		cv_wait(&sem->cond, &sem->mtx);
	}
	sem->counter--;

	mutex_unlock(&sem->mtx); //release mutex lock
}

//increment the value of semaphore s by one
//if there are one or more threads waiting, wake one

void sem_post(struct semaphore* sem)
{
		mutex_lock(&sem->mtx); //make atomic
		
		//increment sem value
		sem->counter++;    

		//wake one up 
		cv_broadcast(&sem->cond);

	    mutex_unlock(&sem->mtx); //release mutex lock
}