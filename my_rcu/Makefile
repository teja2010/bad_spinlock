CFLAGS = -Wall -g -fsanitize=address
#CFLAGS = -O2

all:
	gcc $(CFLAGS) test.c rcu.c -lpthread -o rcu_test
