/*
 * some incorrect spin lock.
 * software locks, without any atomic operations.
 * fun expt. dont use it anywhere ;D
 */
#include "stupid_lock.h"
#include <unistd.h>

/*       The REALLY BAD LOCK
 *the most simple, most stupid lock. single var, true if locked .
 */
struct rbl_struct* really_bad_lock_init()
{
	struct rbl_struct* lock;

	lock = malloc(sizeof(struct rbl_struct));
	lock->really_bad_lock_var = 0; //init as unlocked
	return lock;
}

void really_bad_lock_free(struct rbl_struct *lock)
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

/*end RBL ********************************************************************/

/*     The Cache time lock
 * how long is it taking to get stuff from the cache ?
 */
struct ct_lock* ct_lock_init()
{
	struct ct_lock *lock;
	lock = (struct ct_lock*) malloc(sizeof(struct ct_lock));
	lock->lock = 0;
	return lock;
}
void ct_lock_free(struct ct_lock *lock)
{
	free(lock);
}

void ct_lock(struct ct_lock *lock, struct per_thread_data *ptd)
{
	long int t1=0, t2=0;
	int i;
	struct timespec time[2];
	int len=3000;
	int *one = ptd->hahaha;

again:
	for(i=0; i<len; i++) {
		__builtin_prefetch(&ptd[i], 0,3);
		one[i]=i;
	}
	/* ok, in cache*/

	clock_gettime(CLOCK_REALTIME, &time[0]);
	for(i=0; i<len; i++) {
		if(one[i]==-1)
			break;
	}
	clock_gettime(CLOCK_REALTIME, &time[1]);
	t1 = time[1].tv_nsec - time[0].tv_nsec;

	while(lock->lock == 1);
	lock->lock = 1;
	clock_gettime(CLOCK_REALTIME, &time[0]);
	for(i=0; i<len; i++) {
		if(one[i]==-1)
			break;
	}
	clock_gettime(CLOCK_REALTIME, &time[1]);
	t2 = time[1].tv_nsec - time[0].tv_nsec;

	//printf("%ld %ld\n", t1, t2);
	if(t1 < t2) {
		//oops. u dont have the lock.
		goto again;
	} else
		return;
}

void ct_unlock(struct ct_lock *lock)
{
	lock->lock = 0;
}
/*end CTL     ****************************************************************/

/* Peterson's algorithm
 * * https://www.cse.iitb.ac.in/~mythili/os/notes/notes-swlocks.txt
 * * https://en.wikipedia.org/wiki/Peterson%27s_algorithm

 * * One popular algorithm is Peterson's algorithm. Such algorithms no longer
 * * work correctly on modern multicore systems, but are still useful to
 * * understand the concept of mutual exclusion. Below is an intuitive way of
 * * understanding Peterson algorithm.
 *
 * * Suppose two threads (self and other) are trying to acquire a lock. Consider
 * * the following version of the locking algorithm.
 *
 * * Acquire:
 * * flag[self] = true;
 * * while(flag[other] == true); //busy wait
 *
 * * Release: flag[self] = false;
 *
 * * This algorithm works fine when execution of both threads does not overlap.
 *
 * * Now, consider another locking algorithm.
 *
 * * Acquire:
 * * turn = other;
 * * while(turn == other); //busy wait
 *
 * * This algorithm works fine when execution of both threads overlaps.
 *
 * * So we get a complete algorithm (that works both when executions overlap
 * * and don't) by putting the above two together.
 *
 * * Acquire:
 * * flag[self] = true;
 * * turn = other;
 * * while(flag[other] == true && turn == other); //busy wait
 * *
 * * Release:
 * * flag[self] = false;
 * *
 * * Peterson's algorithm works correctly only if all the writes to flags and
 * * turn happen atomically and in the same order. Modern multicore systems
 * * reorder stores, and this algorithm is not meant to work correctly in
 * * such cases. Modern systems use locks based on atomic hardware instructions
 * * to implement locks.
 */


struct pete2_lock* p2_lock_init()
{
	struct pete2_lock *ptl;

	ptl = (struct pete2_lock*) malloc(sizeof(struct pete2_lock));
	ptl->flags = (bool *) calloc(2, sizeof(bool));
	
	//while(N--)
	//	ptl->flags[N] = false;
	ptl->flags[0] = false;
	ptl->flags[1] = false;
	ptl->turn = -1;

	return ptl;
}

void p2_lock_free(struct pete2_lock *ptl)
{
	free(ptl->flags);
	free(ptl);
}

void p2_lock(struct pete2_lock *ptl, int id)
{
	int other = (id+1)%2;
	ptl->flags[id] = true;
	ptl->turn = other;
	while(ptl->flags[other] == true && ptl->turn == other) { usleep(1); };

	//aquired lock
}

void p2_unlock(struct pete2_lock *ptl, int id)
{
	ptl->flags[id] = false;
}

