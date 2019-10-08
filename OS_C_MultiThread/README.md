### Multithreading in C

In this project we experimented with multi-threading and locks. This project is definitely interesting as findings in this lab is very applicable to Golang.

The project is broken up into 2 parts: project 2 and 2B.

#### In project 2:

We want to find out the optimal number of threads before a race condition occured. We also experimented with using different locks (spin, mutex) and see how multithreading fare with it, and different policy (i.e cooperative vs uncooperative via usign sched_yield() )

##### Learning Points 

1. In this experiment, we found that if the program's task is short i.e the time for each process to complete executing is shorter than the time to create threads, then there won't be any race condition.
2. * For short programs, the throughput from parallellsim is quickly level off as the CPU is saturated i.e CPU is the bottleneck. Furthermore, the overhead of synchronization seems to increase only very slowly. 

   * However, for longer programs i.e longer operations, the throughput continues to drop, as the overhead of synchronization increases with the number of threads. 
   * Since the code inside the critical section does not change with the number of threads, it seems reasonable to assume that the added execution time is being spent getting the locks.
3. There are measurement errors for code performance in multi-threading environment. There are thread creation cost in play, for one. To get an accurate cost of the program's run time, we need to run it sufficiently large iterations such that the thread creation overhead becomes amortized so small that we can ignore it. This value is a good estimate for the "correct cost"
4. Putting locks would slow down performance especially as number of threads increases (the chances of race condition and lock contention also rises).  Thus, more operations are needed to protect shared data, which slows down the program.
5. For program with long operations, having mutex locks could lead to more lock contention. This is because each operation hold the mutex locks longer, leading to higher lock contention costs, which far outweigh the amortization of thread creation when number of threads increases.
6. Spin-locks are terrible, as it keeps polling to check if process is completed, unlike mutex. But, in shorter programs, spin-locks are actually ok.

#### In Project 2B:

We continue to experiment with multi-threading with a stronger emphasis on locks.

##### Learning Point:

1. In high threaded spin-lock execution, most CPU time will be spent spinning, as there will be a lot lock contention.

2. In high threaded mutex execution, most of the CPU time on the execution of the program, but if program is short, most of the time spent will be on context-switches.

3. We also `pprof` the program to determine definitively the run-time of each functions. 

4. If the program, could be broken down into chunks, it could lead to lower lock contention i.e more granulaity of the code

   

| Project | Score Attained |
| ------- | -------------- |
| 2       | 95/100         |
| 2B      | 97/100         |

