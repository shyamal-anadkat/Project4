/* race condition in CV? (must atomically release lock and sleep) */
#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

#define N 1
int ppid;
int pid1, pid2;
volatile int bufsize;
int result;
struct mutex lock;
struct condvar nonfull, nonempty;

#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "race TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

void produce(void *arg);
void consume(void *arg);

int
main(int argc, char *argv[])
{
  int i;
  ppid = getpid();
  cv_init(&nonfull);
  cv_init(&nonempty);

  mutex_init(&lock);
  bufsize = 0;

  assert((pid1 = thread_create(produce, NULL)) > 0);
  assert((pid2 = thread_create(consume, NULL)) > 0);

  for(i = 0; i < 500; i++) {
    result <<= 1;
    sleep(1);
  }

  printf(1, "%p\n", result);
  if(result & 0x3ff)
    printf(1, "race TEST PASSED\n");
  exit();
}

void
produce(void *arg) {
  while(1) {
    mutex_lock(&lock);
    while(bufsize == N)
      cv_wait(&nonfull, &lock);

    result <<= 1;
    result |= 1;

    bufsize++;
    cv_signal(&nonempty);
    mutex_unlock(&lock);
  }
}

void
consume(void *arg) {
  while(1) {
    mutex_lock(&lock);
    while(bufsize == 0)
      cv_wait(&nonempty, &lock);

    result <<= 1;
    result |= 1;

    bufsize--;
    cv_signal(&nonfull);
    mutex_unlock(&lock);
  }
}