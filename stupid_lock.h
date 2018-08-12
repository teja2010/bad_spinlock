#include <stdio.h>
#include <stdlib.h>
/*       The REALLY BAD LOCK
 *the most simple, most stupid lock. single var, true if locked .
 */
struct rbl_struct {
	int really_bad_lock_var;
};
struct rbl_struct* really_bad_lock_init();
struct rbl_struct* really_bad_lock_free(struct rbl_struct *lock);
void really_bad_lock(struct rbl_struct* lock);
void really_bad_unlock();
