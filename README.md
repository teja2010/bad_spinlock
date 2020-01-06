#bad_spinlock

some BAD userspace spinlock implementations

### run the tests:
compilation: `make`

run tests: `./a.out N`   where,  2 < N < 100

or  `sh run_test.sh`
(will run test for 2, 12, 22 ... 102 threads)


### spinlock mutex comparision
 FLASHBACK:
 read: https://probablydance.com/2019/12/30/measuring-mutexes-spinlocks-and-how-bad-the-linux-scheduler-really-is/
 
 torvalds's reply: https://www.realworldtech.com/forum/?threadid=189711&curpostid=189723
 
 further reading:
 
     1. https://lwn.net/Articles/704843/
     2. http://man7.org/linux/man-pages/man3/pthread_spin_init.3.html#NOTES
        User-space spin locks are *NOT* applicable as a general locking
        solution.

```
 lock()       lock acquired                 unlock()
   |----------------|--------------------------|
      wait time (w)      time to work (u)
```

 Assuming irrespective of lock, work done takes equal time, we want to check
 if wait time is different

 ```
 T0 = time it takes a single thread to increment from 0 to M
    = u*M
 ```

 Take a N threads, let each thread increment from 0 to M a variable local to
 the thread within lock protection.
 T = time it takes for all the threads to complete.

 Ideally if wait time is zero,
 ```
 T_i = u * (M*N)  since M*N increments were done, each taking u time

 T_i/T0 = N
 ```

 With a real lock,
 ```
 T_r = W + w*M*N

 W = (T_r - T_i)
   = T_r - N*T0
   where T_r == time it takes to run the test with lock
         N   == number of threads
         T0  == reference number, defined above
 ```

 Table comparing ```(T_0, T_r_spinlock, T_r_mutex)```

| CPU\Threads | 1                  | 2                  | 4                    | 8                      | 16                | 32                 |
|-------------|--------------------|--------------------|----------------------|------------------------|-------------------|--------------------|
| 1           | (1.1, 1.2, 1.1)    | (4.38, 5.3, 4.55)  | (17.78, 33.6, 17.65) | (71.56, 241.46, 71.66) |                   |                    |
| 2           | (0.42, 0.42, 0.42) | (1.42, 1.26, 2.23) | (3.85, 7.89, 8.73)   | (15.92, 63.40, 31.79)  | (67, 670, 128)    |                    |
| 4           | (0.42, 0.42, 0.42) | (0.89, 1.57, 2.6)  | (5.28, 12.23, 23.17) | (7.20, 31.45, 63.5)    | (31.7, 151, 256)  | (267, 2513, 1073)  |
| 6           | (0.42, 0.42, 0.42) | (0.89, 1.50, 4.97) | (3.4, 7.17, 23.17)   | (8.3, 19.9, 94)        | (21.57, 149, 324) | (82, 1872, 1298)   |
| 8           | (0.42, 0.42, 0.42) | (1.55, 2.10, 5.4)  | (3.66, 7.2, 22.93)   |                        | (28, 152, 370)    | (73.1, 1855, 1317) |

Inversion: (Thread/CPU) = (4/1), (8/2), (16/4), (32/8)
Spinlocks are better if Threads/CPU < 4
