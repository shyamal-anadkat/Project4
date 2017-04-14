#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "threads.h"

//****@Author : salil, shyamal****
#define PGSIZE 4096

//THREAD CREATE
int thread_create(void (*thfunc)(void*), void* arg)
{
  
  void *ustack;
  ustack = malloc(2*PGSIZE); //4096 * 2

  //if malloc fails
  if (ustack == 0) {
    return -1; 
  }
  
  //page align if not 
  if((uint)ustack % PGSIZE) {
  	ustack = ustack + (PGSIZE - (uint)ustack % PGSIZE);
  }

  int pid = clone(thfunc, arg, (void*)ustack);
  //check if pid is neg ??

  return pid;
}

// THREAD JOIN
int thread_join(void)
{

  void *ustack;
  int retVal =  join((&ustack));

  //if join fails/pid is negative
  if(retVal == -1) {
    return -1;
  }

  free(ustack);
  return retVal;
}