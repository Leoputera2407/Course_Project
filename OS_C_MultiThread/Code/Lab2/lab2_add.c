#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#define THREADS 't'
#define ITERATIONS 'i'
#define SYNC 'n'
#define YIELD 'y'

#define DEFAULT 'd'
#define MUTEX 'm'
#define SPINLOCK 's'
#define COMPARESWAP 'c'

int nthreads = 1;
static long long counter = 0;
static int niterations = 1;
static int opt_yield = 0;
static int spinlock = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static char syncOp= DEFAULT;

/*void add(long long *pointer, long long value) {
	long long sum = *pointer + value;
	*pointer = sum;
}*/

void add(long long *pointer, long long value) {
        long long sum = *pointer + value;
        if (opt_yield)
        	sched_yield();
        *pointer = sum;
}

void my_add(int value){
	/*Add as many times as iterated*/
	int i=0;
	for(;i<niterations; i++){
		switch(syncOp){
			case DEFAULT:
				add(&counter, value);
				break;
			case MUTEX:
				pthread_mutex_lock(&lock);
				add(&counter, value);
				pthread_mutex_unlock(&lock);
				break;
			case SPINLOCK:
				while(__sync_lock_test_and_set(&spinlock, 1))
					continue;
				add(&counter, value);
				__sync_lock_release(&spinlock);
				break;
			case COMPARESWAP:
				for(long long old,new;;){
					old = counter;
					new = old + value;
					if(__sync_val_compare_and_swap(&counter, old, new) == old)
						break;
				}
				break;
			default:
				break;

		}
	}
}

void* thread_ops(){
	my_add(1);
	my_add(-1);
	return NULL;
}

int main(int argc, char* argv[]){
   int option;
 int long_idx = 0; 	
  struct option long_opt[] = {
    {"threads", required_argument, NULL, THREADS},
    {"iterations", required_argument, NULL, ITERATIONS},
    {"yield", no_argument, NULL, YIELD},
    {"sync", required_argument, NULL, SYNC},
    {0,0,0,0}
  };

  while((option = getopt_long(argc, argv, "", long_opt, &long_idx)) != -1 ){
	switch(option){
		case THREADS:
			nthreads = atoi(optarg);
			break;
		case ITERATIONS:	
			niterations = atoi(optarg);
			break;
		case YIELD:
			opt_yield = 1;
			break;
		case SYNC:
			syncOp = optarg[0];
			if(syncOp != MUTEX && syncOp != SPINLOCK && syncOp != COMPARESWAP){
				fprintf(stderr, "Invalid argument for --sync. Accept 'm', 'c' or 's' only!\n");
				exit(1);
			}
			break;
		default:
			fprintf(stderr,"Invalid argument used.Usage: ./lab2_add --thread=# (default 1) --iterations=# (default 1).\n");
			exit(1);

	}
  }

/*Start Timing*/
 struct timespec starting_time, end_time;
    if (clock_gettime(CLOCK_MONOTONIC, &starting_time) < 0) {
        fprintf(stderr, "clock_gettime() failed. Reason: %s\n", strerror(errno));
        exit(1);
    }

 /*Create threads*/
 pthread_t tids[nthreads];
 int i;
 for(i=0; i<nthreads;i++){
	if(pthread_create(&tids[i], NULL,thread_ops , NULL) != 0){
		fprintf(stderr,"pthread_create() failed. Reason: %s. \n", strerror(errno));
		exit(1);
	}
 }
 
 /*Join threads*/
 int j;
 for(j=0; j <nthreads; j++){
	if(pthread_join(tids[j], NULL) != 0){
		fprintf(stderr, "pthread_join() failed. Reason: %s. \n", strerror(errno));
		exit(1);
	}
 }
 
   /*Clock in the end time */
   if (clock_gettime(CLOCK_MONOTONIC, &end_time) < 0) {
        fprintf(stderr, "Failure in clock_gettime().\n");
	exit(1);
    }
	
   /*Calculate time elapsed*/ 
   long long total_time_elapsed = 1000000000L * (end_time.tv_sec - starting_time.tv_sec) + (end_time.tv_nsec - starting_time.tv_nsec);
   /*Get data for csv*/
   char test[16] = "add";
   if(opt_yield)
	strcat(test, "-yield");

   if(syncOp == DEFAULT)
	strcat(test, "-none");
   else{
	char buf[2];
	sprintf(buf,"-%c",syncOp);
	strcat(test,buf);
   }

   long long nOps = nthreads * niterations * 2;
   long long avgTime = total_time_elapsed / nOps;
   printf("%s,%d,%d,%lld,%lld,%lld,%lld\n",test, nthreads,niterations,nOps, total_time_elapsed, avgTime, counter);
   exit(0);

}

