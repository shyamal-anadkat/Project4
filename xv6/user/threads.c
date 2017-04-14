#include "types.h"
#include "user.h"
#include "fcntl.h"

//****@Author : salil, shyamal****
#define PGSIZE 4096
static struct spinlock tclock;
static struct spinlock tjlock;

//THREAD CREATE
int thread_create(void (*thfunc)(void*), void* arg)
{
  if (!tclock.init) {spin_init(&tclock);}
  //lock
  spin_lock(&tclock);
  
  void *ustack;

  ustack = malloc(1*PGSIZE); //4096 * 1

  //if malloc fails
  if (ustack == 0) {
    spin_unlock(&tclock);
    return -1; 
  }

  int pid = clone(thfunc, arg, (void*)ustack);
  //check if pid is neg ??
  spin_unlock(&tclock);
  return pid;
}

// THREAD JOIN
int thread_join(void)
{
  if (!tjlock.init) {spin_init(&tjlock);}
  spin_lock(&tjlock);

  void *ustack;
  int retVal =  join((&ustack));
  
  //if join fails/pid is negative
  if(retVal == -1) {
    spin_unlock(&tjlock);
    return -1;
  }

  free(ustack);
  spin_unlock(&tjlock);
  return retVal;
}