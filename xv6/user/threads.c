#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "threads.h"

#define PGSIZE 4096

int thread_create(void (*thfunc)(void*), void* arg)
{
  void *ustack;
  ustack = malloc(2*PGSIZE);

  //if malloc fails
  if (ustack == 0) {
    return -1; 
  }
  
  //page align if not 
  if((uint)ustack % PGSIZE) {
  	ustack = ustack + (PGSIZE - (uint)ustack % PGSIZE);
  }

  return (clone(thfunc, arg, (void*)ustack));
}

int thread_join(void)
{
  void *ustack;
  int retVal =  join((&ustack));
  free(ustack);
  return retVal;
}
