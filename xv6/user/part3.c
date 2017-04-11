#include "types.h"
#include "stat.h"
#include "user.h"
 
//struct spinlock *lk;
int status = 0;
struct spinlock lk;
void f(void *arg)
{ 
  // spin_init(&lk);
  //spin_lock(&lk);
  //status = *((int*)arg);

  if(setpark() < 0)
          printf(1, "setpark failed!\n");

  park();  

  int i;
  for(i = 0; i < 10; i++)
  {
      int tmp = status;
      //int j;
      sleep(1);
      status = tmp +1;
  }
  if(unpark(getpid() +1) < 0)
          printf(1, "in thread, unpark failed! pid %d\n", getpid() + 1);

  exit();
//  spin_unlock(&lk);
}
 
int main()
{
  spin_init(&lk);

  int i; 
  for(i = 0; i < 20; i++)
  {
      if(thread_create(&f,(void*)i) < 0)
           printf(2, "clone failed");
  }

  //sleep sort?
  printf(1, "sleeping\n");
  sleep(30);
  printf(1, "awake\n");

  if(unpark(getpid()+1) < 0)
      printf(1, "unpark failed to find a parked thread!\n");

  for(i = 0; i < 20; i++)
  {
      thread_join();
  }

  //thread_create(&f,arg);
  //thread_join();
  //thread_create(&f,arg);
  //thread_join();
 
  printf(1, "status = %d\n", status);

  exit();
}