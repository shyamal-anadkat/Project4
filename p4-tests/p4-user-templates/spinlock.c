
#include "types.h"
#include "user.h"

void spin_init(struct spinlock* lk)
{
	sl->ticket = sl->service = 0;
}

void spin_lock(struct spinlock *lk)
{
	unsigned int myticket = XADD(&sl->ticket, 1);
	while (sl->service != myticket)
		/* spin */;
}

void spin_unlock(struct spinlock *lk)
{
	XADD(&sl->service, 1);
}
