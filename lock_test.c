/* testing all 'em stupid locks.
 * how bad are they?
 *
 * Metric of stupid-ness:
 * 1. start N threads
 * 2. all increment a global var M times, under "protection" (LOL)
 * 3. stupid-ness = (N*M)/(global var value) - 1
 *
 * Under heavy racing, ineffective increments should happen.
 * higher the value, higher stupid-ness.
 */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "stupid_lock.h"

#define THE_M_VALUE  1000000

int protected_var = 0;

/* some common func definitions*/
void create_and_join_threads(int num_threads, void* func(void* arg));


/*test0: good case, ideal case, pass case*/
pthread_spinlock_t *global_pthread_lock;
void* the_correct_spinlock_thread(void *arg)
{
	long int i = THE_M_VALUE;
	printf("s");
	while(i--) {
		pthread_spin_lock(global_pthread_lock);
		protected_var++;
		pthread_spin_unlock(global_pthread_lock);
	}
	printf("d");
}

void the_correct_spinlock_test(int num_threads)
{
	int tcount;
	pthread_t *thread_id;
	double stupidness;
	pthread_spinlock_t lock;

	printf("\n*** The CORRECT SPIN LOCK test\n");
	global_pthread_lock = &lock;
	pthread_spin_init(global_pthread_lock, PTHREAD_PROCESS_PRIVATE);
	create_and_join_threads(num_threads, the_correct_spinlock_thread);
	pthread_spin_destroy(global_pthread_lock);

	stupidness =  (double)(num_threads*THE_M_VALUE);
	stupidness /= (double)     protected_var;
	stupidness -= 1.0;

	printf("test done, stupidness is (%d/%d) - 1 = %f \n",
			num_threads*THE_M_VALUE, protected_var, stupidness);

}

/*test1: The REALLY BAD LOCK */
struct rbl_struct *global_rbl_lock;
void* the_really_bad_lock_thread(void *arg)
{
	long int i = THE_M_VALUE;

	printf("s");
	while(i--) {
		really_bad_lock(global_rbl_lock);
		protected_var++;
		really_bad_unlock(global_rbl_lock);
	}
	printf("d");
}

void the_really_bad_lock_test(int num_threads)
{
	int tcount;
	pthread_t *thread_id;
	double stupidness;

	printf("\n*** The REALLY BAD LOCK test\n");
	global_rbl_lock = really_bad_lock_init();
	create_and_join_threads(num_threads, the_really_bad_lock_thread);
	really_bad_lock_free(global_rbl_lock);

	stupidness =  (double)(num_threads*THE_M_VALUE);
	stupidness /= (double)     protected_var;
	stupidness -= 1.0;

	printf("test done, stupidness is (%d/%d) - 1 = %f \n",
			num_threads*THE_M_VALUE, protected_var, stupidness);
}

int main(int argc, char* argv[])
{
	/* atleast two, please!*/
	int the_N_value = 2;

	if(argc != 2) {
		printf("Just one arg, number of threads.\ne.g. ./test 3\n");
	} else {
		the_N_value = atoi(argv[1]);
		if(the_N_value < 2)
			printf("gud joke, but i'll do it with 2 threads\n");
		else if(the_N_value > 100) {
			printf("Surely You're Joking, Mr. Feynman!\n");
			printf("My machine is old, it can only go till 100\n");
		}
		else
			printf("will run %d threads\n", the_N_value);
	}

	/*comment out if running all of them for large N values is taking long*/

	protected_var = 0;
	the_correct_spinlock_test(the_N_value);

	protected_var = 0;
	the_really_bad_lock_test(the_N_value);

	return 0;
}

/* some utility funcs*/
void create_and_join_threads(int num_threads, void* func(void* arg))
{
	int tcount;
	pthread_t *thread_id;

	// thess commands shud succeed mostly. TODO add checks for them
	thread_id = calloc(num_threads, sizeof(pthread_t));

	for(tcount =0; tcount < num_threads; tcount++){
		pthread_create(&thread_id[tcount], NULL,
				func, NULL);
	}
	printf("C");

	sleep(num_threads/10+1);
	/*join the threads*/
	for(tcount =0; tcount < num_threads; tcount++) {
		pthread_join(thread_id[tcount], NULL);
	}
	printf("J\n");
}
