/* test cv_wait and cv_signal, signal, don't broadcast */
#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;
int global = 0;
struct mutex lock;
struct condvar cond;
int nthreads = 30;

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
   cv_init(&cond);

   mutex_init(&lock);

   int i;
   for(i = 0; i < nthreads; i++) {
     int thread_pid = thread_create(worker, 0);
     assert(thread_pid > 0);
   }

   sleep(50);

   for(i = 0; i < nthreads; i++) {
     mutex_lock(&lock);
     assert(global == i);
     cv_signal(&cond);
     mutex_unlock(&lock);

     int join_pid = thread_join();
     assert(join_pid > 0);

     sleep(10);
     mutex_lock(&lock);
     assert(global == i+1);
     mutex_unlock(&lock);
   }

   printf(1, "cond 2 TEST PASSED\n");
   exit();
}

void
worker(void *arg_ptr) {
  mutex_lock(&lock);
  cv_wait(&cond, &lock);
  global++;
  mutex_unlock(&lock);
  exit();
}