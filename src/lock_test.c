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

#define THE_M_VALUE  10000

int protected_var = 0;
int done_var = 0;

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
	done_var++;

	return arg;
}

void the_correct_spinlock_test(int num_threads)
{
	//int tcount;
	//pthread_t *thread_id;
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
	if(stupidness < 0)
		stupidness *= (-1);

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
	done_var++;

	return arg;
}

void the_really_bad_lock_test(int num_threads)
{
	//int tcount;
	//pthread_t *thread_id;
	double stupidness;

	printf("\n*** The REALLY BAD LOCK test\n");
	global_rbl_lock = really_bad_lock_init();
	create_and_join_threads(num_threads, the_really_bad_lock_thread);
	really_bad_lock_free(global_rbl_lock);

	stupidness =  (double)(num_threads*THE_M_VALUE);
	stupidness /= (double)     protected_var;
	stupidness -= 1.0;
	if(stupidness < 0)
		stupidness *= (-1);

	printf("test done, stupidness is (%d/%d) - 1 = %f \n",
			num_threads*THE_M_VALUE, protected_var, stupidness);
	printf("the_really_bad_lock_test %d,%f\n",num_threads, stupidness);
}

/*test2. the ct lock test*/
struct ct_lock *global_ct_lock;
void* the_ct_lock_thread(void *arg)
{
	struct per_thread_data ptd;
	long int i = THE_M_VALUE;

	printf("s");
	while(i--) {
		ct_lock(global_ct_lock, &ptd);
		protected_var++;
		ct_unlock(global_ct_lock);
	}
	printf("d");
	done_var++;

	return arg;
}

void the_ct_lock_test(int num_threads)
{
	//int tcount;
	//pthread_t *thread_id;
	double stupidness;

	printf("\n*** The CACHE TIME test\n");
	global_ct_lock = ct_lock_init();
	create_and_join_threads(num_threads, the_ct_lock_thread);
	ct_lock_free(global_ct_lock);

	stupidness =  (double)(num_threads*THE_M_VALUE);
	stupidness /= (double)     protected_var;
	stupidness -= 1.0;
	if(stupidness < 0)
		stupidness *= (-1);

	printf("test done, stupidness is (%d/%d) - 1 = %f \n",
			num_threads*THE_M_VALUE, protected_var, stupidness);
	printf("the_cache_time_lock_test %d,%f\n",num_threads, stupidness);
}

/*test 3 peterson's algo*/
struct pete2_lock *global_p2_lock;
void* the_p2_lock_thread(void *arg)
{
	int ID = *(int *)arg;
	long int i = THE_M_VALUE;
	//unsigned int usecs = 0;

	printf("s%d", ID);
	while(i--) {
		p2_lock(global_p2_lock, ID);
		protected_var++;
		p2_unlock(global_p2_lock, ID);
		//usecs = random() % 100;
		//usleep(usecs);
	}
	printf("d");
	done_var++;

	return arg;
}
void the_p2_lock_test(int num_threads)
{
	//int tcount;
	//pthread_t *thread_id;
	double stupidness;

	//printf("\n*** The Peterson's algo test\n");
	global_p2_lock = p2_lock_init();
	create_and_join_threads(num_threads, the_p2_lock_thread);
	p2_lock_free(global_p2_lock);

	stupidness =  (double)(num_threads*THE_M_VALUE);
	stupidness /= (double)     protected_var;
	stupidness -= 1.0;
	if(stupidness < 0)
		stupidness *= (-1);

	printf("test done, stupidness is (%d/%d) - 1 = %f \n",
			num_threads*THE_M_VALUE, protected_var, stupidness);
	printf("the_pete_algo_lock_test %d,%f\n",num_threads, stupidness);
}

/* test 4 : my SIMPle ATomic lock */
struct simple_atomic_lock * global_simpat_lock;
int global_atomic_int;
void* the_simpat_lock_thread(void *arg)
{
	long int i = THE_M_VALUE;

	printf("s");
	while(i--) {
		simpat_lock(global_simpat_lock);
		protected_var++;
		simpat_unlock(global_simpat_lock);
	}

	// // instead do this, increment a atomic global itself.
	// // commenting out since we want to see how stupid it is to use
	// // various *locks* . Adding a test without locks is stupid.
	// // TLDR: wow, no lock, such stupid.
	//
	//struct timespec ts = { 0, 100};
	//while(i--) {
	//	protected_var = __atomic_add_fetch(&global_atomic_int, 1,
	//					   __ATOMIC_RELAXED);
	//	nanosleep(&ts, NULL);
	//}

	printf("d");
	done_var++;

	return arg;
}
void the_simpat_lock_test(int num_threads)
{
	double stupidness;

	printf("\n*** The CORRECT SPIN LOCK test\n");
	global_simpat_lock = simat_lock_init();
	create_and_join_threads(num_threads, the_simpat_lock_thread);

	stupidness =  (double)(num_threads*THE_M_VALUE);
	stupidness /= (double)     protected_var;
	stupidness -= 1.0;
	if(stupidness < 0)
		stupidness *= (-1);

	printf("test done, stupidness is (%d/%d) - 1 = %f \n",
			num_threads*THE_M_VALUE, protected_var, stupidness);
	printf("the_simpat_lock_test %d,%f\n",num_threads, stupidness);
}


int main(int argc, char* argv[])
{
	/* atleast two, please!*/
	int the_N_value = 2;
	protected_var = 0;

	if(argc != 2) {
		printf("Just one arg, number of threads.\ne.g. ./test 3\n");
	} else {
		the_N_value = atoi(argv[1]);
		if(the_N_value < 2) {
			printf("gud joke, but i'll do it with 2 threads\n");
			the_N_value = 2;
		} else if(the_N_value > 100) {
			printf("Surely You're Joking, Mr. Feynman!\n");
			printf("My machine is old, it can only go till 100\n");
			the_N_value = 100;
		} else
			printf("will run %d threads\n", the_N_value);
	}

	/*comment out if running all of them for large N values is taking long*/

	the_correct_spinlock_test(the_N_value);

	the_really_bad_lock_test(the_N_value);

//	the_ct_lock_test(the_N_value);

	the_p2_lock_test(the_N_value);

	the_simpat_lock_test(the_N_value);

	return 0;
}

/* some utility funcs*/
void create_and_join_threads(int num_threads, void* func(void* arg))
{
	int tcount;
	pthread_t *thread_id;
	int *ID;

	// thess commands shud succeed mostly. TODO add checks for them
	thread_id = calloc(num_threads, sizeof(pthread_t));
	ID = calloc(num_threads, sizeof(int));

	for(tcount =0; tcount < num_threads; tcount++) {
		ID[tcount] = tcount;
		pthread_create(&thread_id[tcount], NULL,
				func, (void*)&ID[tcount]);
	}
	printf("C");

	while (done_var <  0.9 * num_threads) { /*rather optimistic :D */
		sleep(1);
		printf("w\n");
	}

	/*join the threads*/
	for(tcount =0; tcount < num_threads; tcount++) {
		pthread_join(thread_id[tcount], NULL);
	}
	printf("J\n");
}

