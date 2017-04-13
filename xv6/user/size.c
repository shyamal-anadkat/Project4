/* check that address space size is updated in threads */
#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;
int global = 0;
unsigned int size = 0;
struct mutex mutex, mutex2;
int num_threads = 30;


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

   int arg = 101;
   void *arg_ptr = &arg;

   mutex_init(&mutex);
   mutex_init(&mutex2);
   mutex_lock(&mutex);
   mutex_lock(&mutex2);

   int i;
   for (i = 0; i < num_threads; i++) {
      int thread_pid = thread_create(worker, arg_ptr);
      assert(thread_pid > 0);
   }

   size = (unsigned int)sbrk(0);

   while (global < num_threads) {
      mutex_unlock(&mutex);
      sleep(100);
      mutex_lock(&mutex);
   }

   global = 0;
   sbrk(10000);
   size = (unsigned int)sbrk(0);
   mutex_unlock(&mutex);

   while (global < num_threads) {
      mutex_unlock(&mutex2);
      sleep(100);
      mutex_lock(&mutex2);
   }
   mutex_unlock(&mutex2);


   for (i = 0; i < num_threads; i++) {
      int join_pid = thread_join();
      assert(join_pid > 0);
   }

   printf(1, "TEST PASSED\n");
   exit();
}

void
worker(void *arg_ptr) {
   mutex_lock(&mutex);
   assert((unsigned int)sbrk(0) == size);
   global++;
   mutex_unlock(&mutex);

   mutex_lock(&mutex2);
   assert((unsigned int)sbrk(0) == size);
   global++;
   mutex_unlock(&mutex2);

   exit();
}
