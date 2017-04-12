#ifndef SPINLOCK_H
#define SPINLOCK_H

struct spinlock {
	uint flag;
};

void spin_init(struct spinlock* lk);
void spin_lock(struct spinlock *lk);
void spin_unlock(struct spinlock *lk);

#endif
