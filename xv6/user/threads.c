#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "threads.h"

#define PGSIZE 4096

int thread_create(void (*thfunc)(void*), void* arg)
{
  
  /* fill this in! */
  void *ustack;
  ustack = malloc(2*PGSIZE); // twice pagesize ???
  
  //page align if not 
  if((uint)ustack % PGSIZE) {
  	ustack = ustack + (PGSIZE - (uint)ustack % PGSIZE);
  }
  return (clone(thfunc, arg, ustack));
}

int thread_join(void)
{
   /* fill this in! */
  void *ustack;
  int retVal =  join((&ustack));
  free(ustack);
  return retVal;
}
