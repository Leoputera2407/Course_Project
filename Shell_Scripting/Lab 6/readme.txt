The objective of this project was to parallize the ray tracer code.

Difficulties:
1) It was difficult to figure out which block of code to parallize ie the third argument for the create_thread function.
   I soon figured there was one segment of code which colorize the pixels, and parallized that. 
2) After I parallize it, I realized myoutput wasn't speeding up as much. 
Then, I realize I should have iterate the for loop differently, jumping by nthreads each time to really capture the speed up. 
This way of iterating is faster too, because it minimizes cache misses as each threads
can use the what's in the cache.
3) Furthermore, my output is a bit wrong. Then, I realize that the RGB components need to be put in order, hence why I put it one by one.


Conclusion:
The ray tracer real time execution improve proportionally every time a new thread was added.

1 threads
real	0m47.611s
user	0m47.599s
sys		0m0.003s

2 threads
real	0m24.086s
user	0m48.131s
sys		0m0.001s

4 threads
real	0m12.213s
user	0m48.238s
sys		0m0.004s

8 threads
real	0m6.461s
user	0m50.134s
sys		0m0.004s

Every time the threads are doubled the real time execution decreases roughly by half. 

