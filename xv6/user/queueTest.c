
#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
  
	struct Queue* q;
	q = initQueue(2);

	printf(1,"q init done\n");

	enqueue(q, 1000);
	enqueue(q, 2000);
	enqueue(q, 3000);



	printf(1, "1) %d dequeued from queue\n", dequeue(q));

	printf(1, "is empty @start? : %d \n", isEmpty(q));

	printf(1, "2) %d dequeued from queue\n", dequeue(q));
	printf(1, "3) %d dequeued from queue\n", dequeue(q));

	printf(1, "is empty after 3 dq? : %d \n", isEmpty(q));
	
	printf(1, "4) %d dequeued from queue\n", dequeue(q));


  exit();
}