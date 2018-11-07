#include <stdio.h>
#include <stdlib.h>
#include<stdio.h>
#include<time.h>
#include<string.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdbool.h>

/*       The REALLY BAD LOCK
 *the most simple, most stupid lock. single var, true if locked .
 */
struct rbl_struct {
	int really_bad_lock_var;
};
struct rbl_struct* really_bad_lock_init();
void really_bad_lock_free(struct rbl_struct *lock);
void really_bad_lock(struct rbl_struct* lock);
void really_bad_unlock();

/*        The ID lock
 */

/*     was I prempted? clocks CLOCK_THREAD_CPUTIME_ID check
 */
/*     The Cache time lock
 * how long is it taking to get stuff from the cache ?
 */
struct ct_lock {
	int lock;
};
struct per_thread_data {
	int hahaha[3000];	//hahaha
};
struct ct_lock* ct_lock_init();
void ct_lock_free(struct ct_lock *lock);
void ct_lock(struct ct_lock *lock, struct per_thread_data *ptd);
void ct_unlock(struct ct_lock *lock);


/*
 * perterson's algo
 */
struct pete2_lock {
	bool *flags;
	int turn;
};
struct pete2_lock* p2_lock_init();
void p2_lock_free(struct pete2_lock *ptl);
void p2_lock(struct pete2_lock *ptl, int id);
void p2_unlock(struct pete2_lock *ptl, int id);
