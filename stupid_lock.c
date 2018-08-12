/*
 * some incorrect spin lock.
 * software locks, without any atomic operations.
 * fun expt. dont use it anywhere ;D
 */
#include "stupid_lock.h"

struct rbl_struct* really_bad_lock_init()
{
	struct rbl_struct* lock;

	lock = malloc(sizeof(struct rbl_struct));
	lock->really_bad_lock_var = 0; //init as unlocked
	return lock;
}

struct rbl_struct* really_bad_lock_free(struct rbl_struct *lock)
{
	free(lock);
}

void really_bad_lock(struct rbl_struct* lock)
{
	while(lock->really_bad_lock_var == 1);
	lock->really_bad_lock_var = 1;
}

void really_bad_unlock(struct rbl_struct* lock)
{
	lock->really_bad_lock_var = 0;
}
