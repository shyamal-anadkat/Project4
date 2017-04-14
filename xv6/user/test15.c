/* test lock correctness */
#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;
int global = 0;
struct mutex lock;
int num_threads = 30;
int loops = 1000;


#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "locks.c TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

void worker(void *arg_ptr);

int
main(int argc, char *argv[])
{
   ppid = getpid();

   mutex_init(&lock);

   int i;
   for (i = 0; i < num_threads; i++) {
      int thread_pid = thread_create(worker, 0);
      assert(thread_pid > 0);
   }

   for (i = 0; i < num_threads; i++) {
      int join_pid = thread_join();
      assert(join_pid > 0);
   }

   assert(global == num_threads * loops);

   printf(1, "locks TEST PASSED\n");
   exit();
}

void
worker(void *arg_ptr) {
   int i, j, tmp;
   for (i = 0; i < loops; i++) {
      mutex_lock(&lock);
      tmp = global;
      for(j = 0; j < 50; j++); // take some time
      global = tmp + 1;
      mutex_unlock(&lock);
   }
   exit();
}
