#ifndef __MY_RCU_RCU_INCLUDE
#define __MY_RCU_RCU_INCLUDE
#include <stdint.h>
#include <pthread.h>

struct rcu_data {
	struct rcu_node *h;
	pthread_spinlock_t *lock;
	void (*func) (void*);
};

struct rcu_node {
	void *ptr;
	uint32_t count;
	int head;
};

void rcu_init(struct rcu_data *global, void (*func) (void*));
void set_rcu_call(struct rcu_data *global, void (*func) (void*));

// reader
// read lock will return a handle
int rcu_read_lock(struct rcu_data *global, struct rcu_node **handle);
void rcu_read_unlock(struct rcu_data *global, struct rcu_node *handle);
void* rcu_dereference(struct rcu_data *global, struct rcu_node *handle);

// writer
int rcu_assign_pointer_cas(struct rcu_data *global, void *val, void *oldval, int len);

#endif

