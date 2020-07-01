#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rcu.h"

void rcu_init(struct rcu_data *global, void (*func) (void*))
{
	global->h = NULL;

	global->lock = (pthread_spinlock_t *)
				calloc(1, sizeof(pthread_spinlock_t));
	if(pthread_spin_init(global->lock, PTHREAD_PROCESS_PRIVATE) == 1) {
		printf("pthread_spin_init failed\n");
		exit(-1);
	}

	if (!func)
		func = free;
	global->func = func;
}

int rcu_read_lock(struct rcu_data *global, struct rcu_node **handle)
{
	int ret = -1;

	pthread_spin_lock(global->lock);
	do {
		*handle = global->h;
		if (global->h == NULL || global->h->count<0) {
			break;
		}

		global->h->count++;
		ret = 0;
	} while(0);
	pthread_spin_unlock(global->lock);

	return ret;
}

/* zero => obj is freed
 */
void rcu_read_unlock(struct rcu_data *global, struct rcu_node *handle)
{
	pthread_spin_lock(global->lock);
	handle->count--;

	if(handle->count == 0 && handle->head == 0) {
		global->func(handle->ptr); // rcu_call()
		free(handle);
	}

	pthread_spin_unlock(global->lock);
	return;
}

void* rcu_dereference(struct rcu_data *global, struct rcu_node *handle)
{
	void *ret = NULL;

	pthread_spin_lock(global->lock);
	ret = handle->ptr;
	pthread_spin_unlock(global->lock);

	return ret;
}

int rcu_assign_pointer_cas(struct rcu_data *global,
			   void *val, void *oldval, int len)
{
	int ret = -1;
	pthread_spin_lock(global->lock);

	do {
		if(global->h == NULL && oldval !=NULL) {
			break;
		}

		if (global->h != NULL &&
		    memcmp(global->h->ptr, oldval, len) != 0) {
			break;
		}

		if(global->h != NULL) {
			global->h->head = 0;
			printf("CAS count %d\n",global->h->count);
			if (global->h->count == 0) {
				global->func(global->h->ptr);
				free(global->h);
			}
		}

		global->h = calloc(1, sizeof(struct rcu_node));
		global->h->ptr = val;
		global->h->head = 1;
		ret = 0;
	} while(0);

	pthread_spin_unlock(global->lock);
	return ret;
}

