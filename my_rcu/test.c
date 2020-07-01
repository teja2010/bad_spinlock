#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include "rcu.h"

#define N_ITERS 10000

struct rcu_data global; //the permanent variable

struct current_ptr {
	int x_min, x_max;
	int y_min, y_max;
	int z_min, z_max;
};

void* reader_thread(void *arg)
{
	struct current_ptr *p = NULL;
	struct rcu_node *handle = NULL;
	int run = 1;
	int last = 0;

	while(run) {
		int i = N_ITERS;
		if (rcu_read_lock(&global, &handle) <0) {
			printf("read_lock failed\n");
			usleep(100);
			continue;
		}
		p = (struct current_ptr*) rcu_dereference(&global, handle);
		if (p->y_min >= N_ITERS)
			run = 0;
		last = p->z_min;

		while(i) {
			if (p->x_min > 0)
				i-= p->x_min;
			i--;
		}
		rcu_read_unlock(&global, handle);
	}

	printf("last saw %d\n", last);
	return NULL;
}

void* writer_thread(void *arg)
{
	struct current_ptr *p = NULL;
	struct current_ptr old;
	struct current_ptr *temp = NULL;
	int run = 1;
	int set = 0;

	while(run) {
		struct rcu_node *handle = NULL;
		if(!temp)
			temp = calloc(1, sizeof(struct current_ptr));

		if (rcu_read_lock(&global, &handle) < 0) {
			printf("read_lock failed\n");
			continue;
		}

		p = (struct current_ptr*) rcu_dereference(&global, handle);
		memcpy(&old, p, sizeof(old));
		memcpy(temp, p, sizeof(*temp));
		rcu_read_unlock(&global, handle);

		// some random changes
		temp->y_min++;
		temp->z_min++;
		temp->x_min = 1;
		set = temp->z_min;
		if (set > N_ITERS)
			run = 0;

		if(rcu_assign_pointer_cas(&global, temp, &old,
					  sizeof(struct current_ptr))) {
			continue;
		} else {
			temp = NULL;
		}
		usleep(30);
	}

	free(temp);
	printf("SET LAST %d\n", set);

	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t *thread_ids = NULL;
	int num_readers = 2, num_writers = 1;
	int i  = 0, j = 0;

	if(argc != 3) {
		printf("Usage:\ne.g. ./rcu_test <reader_threads> <writer_threads>\n");
		return 0;
	} else {
		num_readers = atoi(argv[1]);
		if(num_readers < 2) {
			printf("start 2 reader threads\n");
			num_readers = 2;
		} else
			printf("will run %d reader threads\n", num_readers);

		num_writers = atoi(argv[2]);
		if(num_writers < 1) {
			printf("start 1 writer threads\n");
			num_writers = 1;
		} else
			printf("will run %d writer threads\n", num_writers);
	}

	rcu_init(&global, NULL);
	struct current_ptr *ptr = calloc(1, sizeof(struct current_ptr));
	if(rcu_assign_pointer_cas(&global, ptr, NULL,
				  sizeof(struct current_ptr)) < 0) {
		printf("assign_ptr_cas failed\n");
		return -1;
	}

	thread_ids = calloc(num_readers + num_writers, sizeof(pthread_t));
	if (!thread_ids) {
		printf("calloc failed\n");
		return 0;
	}

	while(i< num_readers || j < num_writers) {
		if (j<num_writers) {
			int rc = pthread_create(&thread_ids[i+j], NULL,
						writer_thread, NULL);
			if (rc < 0) {
				printf("pthread_create err: %m");
			}
			j++;
		}

		if (i<num_readers) {
			int rc = pthread_create(&thread_ids[i+j], NULL,
						reader_thread, NULL);
			if (rc < 0) {
				printf("pthread_create err: %m");
			}
			i++;
		}
	}

	for(i=0; i < num_readers+num_writers; i++) {
		int rc = pthread_join(thread_ids[i], NULL);
		if (rc < 0) {
			printf("pthread_join err: %m");
		}
	}

	free(thread_ids);
}

