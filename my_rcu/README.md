### my rcu

Mix of object counters and rcu. No quiescent state, free the object once the counter goes to zero.

Each read needs three lock()/unlock() calls. Good only if the read period is long...
