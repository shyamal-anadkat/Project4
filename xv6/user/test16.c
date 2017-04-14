/* test cv_wait and cv_signal, cannot leave cv_wait without lock */
#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;
int global = 0;
struct mutex lock;
struct condvar cond;

#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

void worker(void *arg_ptr);

int
main(int argc, char *argv[])
{
   ppid = getpid();

   printf(1, "before mutex_init\n");
   mutex_init(&lock);

   printf(1, "before thread_create\n"); 
   int thread_pid = thread_create(worker, 0);
   assert(thread_pid > 0);
   printf(1, "after thread_create\n"); 

   sleep(2);
   printf(1, "before mutex_LOCK\n");
   mutex_lock(&lock);

   global = 2;
   printf(1, "before CV_SIGNAL\n");
   cv_signal(&cond);
   printf(1, "after CV_SIGNAL\n");
   sleep(5);
   global = 1;
   mutex_unlock(&lock);

   printf(1, "BEFORE thread_join\n");
   int join_pid = thread_join();
   assert(join_pid == thread_pid);

   printf(1, "cond 1 TEST PASSED\n");
   exit();
}

void
worker(void *arg_ptr) {
  printf(1, "INSIDE WORKER_START\n");
  mutex_lock(&lock);
  assert(global == 0);
  cv_wait(&cond, &lock);
  assert(global == 1);
  mutex_unlock(&lock);
   printf(1, "WORKER DONE\n");
  exit();
}
