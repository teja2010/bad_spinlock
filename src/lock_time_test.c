/*
 * FLASHBACK:
 * read: https://probablydance.com/2019/12/30/measuring-mutexes-spinlocks-and-how-bad-the-linux-scheduler-really-is/
 * torvalds's reply: https://www.realworldtech.com/forum/?threadid=189711&curpostid=189723
 * further reading:
 *     1. https://lwn.net/Articles/704843/
 *     2. http://man7.org/linux/man-pages/man3/pthread_spin_init.3.html#NOTES
 *        User-space spin locks are *NOT* applicable as a general locking
 *        solution.
 *
 * lock()       lock acquired                 unlock()
 *   |----------------|--------------------------|
 *      wait time (w)      time to work (u)
 *
 * Assuming irrespective of lock, work done takes equal time, we want to check
 * if wait time is different
 *
 * T0 = time it takes a single thread to increment from 0 to M
 *    = u*M
 *
 * Take a N threads, let each thread increment from 0 to M a variable local to
 * the thread within lock protection.
 * T = time it takes for all the threads to complete.
 *
 * Ideally if wait time is zero,
 * T_i = u * (M*N)  since M*N increments were done, each taking u time
 *
 * T_i/T0 = N
 *
 * With a real lock,
 * T_r = W + w*M*N
 *
 * W = (T_r - T_i)
 *   = T_r - N*T0
 *   where T_r == time it takes to run the test with lock
 *         N   == number of threads
 *         T0  == reference number, defined above
 *
 */
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int num_threads = 2;
int done = 0;

/* some utility funcs*/
#define M_VALUE  200000
#define the_standard_loop()					\
	do {							\
		int i=1, j=1;					\
		while(i%M_VALUE != 0) {				\
			while(j%M_VALUE != 0) {			\
				j++;				\
			}					\
			j=1; i++;				\
		}						\
	} while(0);

/* funcs similar to the ones in time.h */
# define timerspec_clear(tvp)	((tvp)->tv_sec = (tvp)->tv_nsec = 0)
# define timerspec_debug(s,a)	printf("%s%lu.%lu", s, (a)->tv_sec, (a)->tv_nsec);
# define timerspec_print(s,a)	printf("%s %lu.%lu\n", s, (a)->tv_sec, (a)->tv_nsec);
# define timerspec_set(a,b)					\
	do {							\
		(a)->tv_sec = (b)->tv_sec;			\
		(a)->tv_nsec = (b)->tv_nsec;			\
	} while(0);

# define timerspec_cmp(a, b, CMP)	/* return True if a CMP b */          \
  (((a)->tv_sec == (b)->tv_sec) ? 					      \
   ((a)->tv_nsec CMP (b)->tv_nsec) : 					      \
   ((a)->tv_sec CMP (b)->tv_sec))

# define timerspec_sub(a, b, result)					      \
  do {									      \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;			      \
    (result)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec;			      \
    if ((result)->tv_nsec < 0) {					      \
      --(result)->tv_sec;						      \
      (result)->tv_nsec += 1000000000;					      \
    }									      \
  } while (0)
# define timerspec_add(a, b, result)					      \
  do {									      \
    (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;			      \
    (result)->tv_nsec = (a)->tv_nsec + (b)->tv_nsec;			      \
    if ((result)->tv_nsec >= 1000000000)				      \
      {									      \
	++(result)->tv_sec;						      \
	(result)->tv_nsec -= 1000000000;				      \
      }									      \
  } while (0)


void create_and_join_threads(void* func(void *arg))
{
	int tcount;
	pthread_t *thread_id = NULL;
	struct timespec *tp_arr, tp_sum;
	struct timespec tp_d;
	//struct timespec *tp1_min = NULL, *tp2_max = NULL;

	// these commands shud succeed mostly. TODO add checks for them
	thread_id = calloc(num_threads, sizeof(pthread_t));
	tp_arr = calloc(2*num_threads, sizeof(struct timespec));

	for(tcount =0; tcount < num_threads; tcount++) {
		pthread_create(&thread_id[tcount], NULL,
				func, (void*)(tp_arr + 2*tcount));
	}

	/*join the threads*/
	for(tcount =0; tcount < num_threads; tcount++) {
		pthread_join(thread_id[tcount], NULL);
	}

	/* compute max(tp2) - min(tp1)  */
	//tp1_min = &tp_arr[0];
	//tp2_max = &tp_arr[1];
	timerspec_clear(&tp_sum);

	for(tcount =0; tcount < num_threads; tcount++) {
		timerspec_clear(&tp_d);
		timerspec_sub(&tp_arr[tcount*2+1], &tp_arr[tcount*2], &tp_d);
		timerspec_debug(",", &tp_d);
		timerspec_add(&tp_sum, &tp_d, &tp_sum);

		//if(timerspec_cmp(&tp_arr[tcount*2], tp1_min, <))
		//	timerspec_set(tp1_min, &tp_arr[tcount*2]);

		//if(timerspec_cmp(&tp_arr[tcount*2+1], tp2_max, >))
		//	timerspec_set(tp2_max, &tp_arr[tcount*2+1]);
	}
	//timerspec_debug("tp1_min", tp1_min);
	//timerspec_debug("tp2_max", tp2_max);

	//timerspec_clear(&tp_diff);
	//timerspec_sub(tp2_max, tp1_min, &tp_diff);
	//timerspec_debug("tp_diff", &tp_diff);
	timerspec_print("\t tp_sum=", &tp_sum);

	free(thread_id);
	free(tp_arr);
}

void* func_measure_T0(void *farg)
{
	struct timespec *tp_arr = (struct timespec *)farg;
	struct timespec *tp1 = NULL, *tp2 = NULL;

	tp1 = &tp_arr[0];
	tp2 = &tp_arr[1];

	clock_gettime(CLOCK_REALTIME, tp1);
	do {
		int i=1, j=1;
		while(i%M_VALUE != 0) {
			// no lock
			while(j%1000!= 0) {
				j++;
			}
			j=1; i++;
			// no unlock
		}
	} while(0);
	clock_gettime(CLOCK_REALTIME, tp2);

	return NULL;
}

void measure_T0()
{
	printf("T0 value:\n");
	create_and_join_threads(func_measure_T0);
}

pthread_spinlock_t *global_pthread_lock;
void* func_measure_Tr_pthread_spinlock(void *farg)
{
	struct timespec *tp_arr = (struct timespec *)farg;
	struct timespec *tp1 = NULL, *tp2 = NULL;

	tp1 = &tp_arr[0];
	tp2 = &tp_arr[1];

	clock_gettime(CLOCK_REALTIME, tp1);
	do {
		int i=1, j=1;
		while(i%M_VALUE != 0) {
			pthread_spin_lock(global_pthread_lock);
			while(j%1000 != 0) {
				j++;
			}
			j=1; i++;
			pthread_spin_unlock(global_pthread_lock);
		}
	} while(0);
	clock_gettime(CLOCK_REALTIME, tp2);

	return NULL;
}

void measure_Tr_pthread_spinlock()
{
	pthread_spinlock_t lock;

	printf("\n*** The PTHREAD SPINLOCK LOCK test\n");
	global_pthread_lock = &lock;
	pthread_spin_init(global_pthread_lock, PTHREAD_PROCESS_PRIVATE);
	create_and_join_threads(func_measure_Tr_pthread_spinlock);
	pthread_spin_destroy(global_pthread_lock);
}

pthread_mutex_t global_pthread_mutex;
void* func_measure_Tr_pthread_mutex(void *farg)
{
	struct timespec *tp_arr = (struct timespec *)farg;
	struct timespec *tp1 = NULL, *tp2 = NULL;

	tp1 = &tp_arr[0];
	tp2 = &tp_arr[1];

	clock_gettime(CLOCK_REALTIME, tp1);
	do {
		int i=1, j=1;
		while(i%M_VALUE != 0) {
			pthread_mutex_lock(&global_pthread_mutex);
			while(j%1000 != 0) {
				j++;
			}
			j=1; i++;
			pthread_mutex_unlock(&global_pthread_mutex);
		}
	} while(0);
	clock_gettime(CLOCK_REALTIME, tp2);

	return NULL;
}

void measure_Tr_pthread_mutex()
{
	pthread_spinlock_t lock;

	printf("\n*** The PTHREAD MUTEX LOCK test\n");
	global_pthread_lock = &lock;
	pthread_mutex_init(&global_pthread_mutex, NULL);
	create_and_join_threads(func_measure_Tr_pthread_mutex);
	pthread_mutex_destroy(&global_pthread_mutex);
}
int main(int argc, char* argv[])
{
	if(argc != 2) {
		printf("Just one arg, number of threads.\ne.g. ./test 3\n");
	} else {
		num_threads = atoi(argv[1]);
		if(num_threads > 100) {
			printf("My machine is old, it can only go till 100\n");
			num_threads = 100;
		} else
			printf("will run %d threads\n", num_threads);
	}

	measure_T0();//hot hot
	measure_T0();
	measure_T0();
	measure_Tr_pthread_spinlock();
	measure_Tr_pthread_spinlock();
	measure_Tr_pthread_spinlock();
	measure_Tr_pthread_mutex();
	measure_Tr_pthread_mutex();
	measure_Tr_pthread_mutex();
}
