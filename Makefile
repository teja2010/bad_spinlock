
all:
	gcc -Wall -g src/stupid_lock.c src/lock_test.c -pthread -o stupid_locks
	gcc -Wall -g src/lock_time_test.c -pthread -o lock_time_test

clean:
	rm stupid_locks lock_time_test
