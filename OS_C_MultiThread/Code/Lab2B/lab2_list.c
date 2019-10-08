#include <getopt.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include "SortedList.h"


#define THREADS     't'
#define ITERATIONS  'i'
#define SYNC        'z'
#define YIELD       'y'
#define LIST 	    'l'

#define DEFAULT     'd'
#define MUTEX       'm'
#define SPINLOCK    's'


#define ID_YIELD        0x03  
#define IL_YIELD        0x05  
#define DL_YIELD        0x06  
#define IDL_YIELD       0x07  

static int randomKeyLength = 5;
char possible_keys[53] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

int opt_yield = 0;

int nthreads = 1;
static int niterations          = 1;
static int nlists               = 1;
static unsigned int nelements   = 0;
static char syncOps             = DEFAULT;

int* spinlocks             = NULL;
static pthread_mutex_t* locks = NULL;
static SortedList_t* lists = NULL;
static SortedListElement_t *element_arr = NULL;
static long long* wait_time = NULL;


void lists_locks_init(){	
    lists = (SortedList_t*) malloc(sizeof(SortedList_t) * nlists);
    if(syncOps == MUTEX)
    	locks = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t) * nlists);
    else if(syncOps == SPINLOCK)
	spinlocks= (int *) malloc(sizeof(int) * nlists);

    for(int i=0; i <nlists; i++){ 
    	 SortedList_t* head= &lists[i]; 
	 lists[i].key=NULL;
         lists[i].next=head;
	 lists[i].prev=head;
	 if(syncOps == MUTEX){
	 	pthread_mutex_init(&locks[i],NULL);
	 }else if(syncOps == SPINLOCK)
		spinlocks[i] = 0;
    }
}

//  http://stackoverflow.com/questions/7666509/hash-function-for-string
int determine_list(const char *key){
    unsigned long hash = 5381;
    for (int i = 0; i < randomKeyLength; i++)
        hash = ((hash << 5) + hash) + key[i]; 
    unsigned int ret = hash % nlists;
    return ret;
}

void elem_init() {
    srand((unsigned int) time(NULL));
    for (unsigned int i = 0; i < nelements; i++) {
	char* key = malloc(sizeof(char) * (randomKeyLength +1));
	for(int j=0;j<randomKeyLength;j++)
		key[j]= possible_keys[rand()%52];
        key[randomKeyLength] = '\0';
        element_arr[i].key = key;
    }
}

void free_everything(){
	free(spinlocks);
	free(locks);
	free(lists);
	free(element_arr);
	free(wait_time);
}

void wait_time_init(){
	wait_time = malloc(nthreads* sizeof(long long));
	for(int i=0; i<nthreads; i++)
		wait_time[i]=0;
}

void checkAndDelete(SortedList_t* sublist,  int elem_idx){
	SortedListElement_t *elem = SortedList_lookup(sublist, element_arr[elem_idx].key);
	if(elem == NULL || SortedList_delete(elem) != 0){
		fprintf(stderr,"List is corrupted. \n");
		exit(2);
	}
}

void checkIfSpinLock(int sublist_idx){
	while(__sync_lock_test_and_set(&(spinlocks[sublist_idx]),1))
		continue;
}	

int getTotalLength(){
	int list_len =0;
	for(int i=0; i<nlists; i++){
              int sublist_len = SortedList_length(&lists[i]);
	      if(sublist_len <0){
			fprintf(stderr,"List is corrupted\n");
			exit(2);
		}
	list_len +=sublist_len;
	}
	return list_len;
}

int getMutexWaitTime(){
	struct timespec elapsed_start, elapsed_end;
    	if(clock_gettime(CLOCK_MONOTONIC, &elapsed_start) <0 ){
	 	fprintf(stderr,"Failed to clock_gettime(). Reason: %s. \n", strerror(errno));
	 	exit(1);
    	}
	//Measure wait time for all locks
	for(int i=0; i< nlists;i++)
		pthread_mutex_lock(&locks[i]);				
    	if(clock_gettime(CLOCK_MONOTONIC, &elapsed_end) <0 ){
	 	fprintf(stderr,"Failed to clock_gettime(). Reason: %s. \n", strerror(errno));
	 	exit(1);
    	}
	return ((elapsed_end.tv_sec - elapsed_start.tv_sec) * 1000000000LL + (elapsed_end.tv_nsec - elapsed_start.tv_nsec));
}


void* thread_ops(void* start_pos){
	int begin_pos = *((int*) start_pos);
	int thread_id = begin_pos / niterations;
  	SortedList_t* sublist =NULL;
	for(int j=begin_pos; j < begin_pos + niterations; j++){
    		struct timespec elapsed_start, elapsed_end;
		int sublist_idx= determine_list(element_arr[j].key);
		sublist=&lists[sublist_idx];
		switch(syncOps){
			case DEFAULT:
				SortedList_insert(sublist,&element_arr[j]);
				break;
			case MUTEX:
    				//Start time
    				if(clock_gettime(CLOCK_MONOTONIC, &elapsed_start) <0 ){
	 				fprintf(stderr,"Failed to clock_gettime(). Reason: %s. \n", strerror(errno));
					exit(1);
    				}
				pthread_mutex_lock(&locks[sublist_idx]);				
    				if(clock_gettime(CLOCK_MONOTONIC, &elapsed_end) <0 ){
	 				fprintf(stderr,"Failed to clock_gettime(). Reason: %s. \n", strerror(errno));
					exit(1);
    				}
				wait_time[thread_id] += (elapsed_end.tv_sec - elapsed_start.tv_sec) * 1000000000LL + (elapsed_end.tv_nsec - elapsed_start.tv_nsec);
				SortedList_insert(sublist, &element_arr[j]);
				pthread_mutex_unlock(&locks[sublist_idx]);
				break;
			case SPINLOCK:
    				if(clock_gettime(CLOCK_MONOTONIC, &elapsed_start) <0 ){
	 				fprintf(stderr,"Failed to clock_gettime(). Reason: %s. \n", strerror(errno));
					exit(1);
    				}
				checkIfSpinLock(sublist_idx);
    				if(clock_gettime(CLOCK_MONOTONIC, &elapsed_end) <0 ){
	 				fprintf(stderr,"Failed to clock_gettime(). Reason: %s. \n", strerror(errno));
					exit(1);
    				}
				wait_time[thread_id] += (elapsed_end.tv_sec - elapsed_start.tv_sec) * 1000000000LL + (elapsed_end.tv_nsec - elapsed_start.tv_nsec);
				SortedList_insert(sublist, &element_arr[j]);
				__sync_lock_release(&spinlocks[sublist_idx]);
				break;
			default:
				break;
		}
	}
	int list_len=0;
	switch(syncOps){
        	case DEFAULT:
			list_len = getTotalLength();
                       	 break;
                case MUTEX:
			 wait_time[thread_id] += getMutexWaitTime();
			 list_len = getTotalLength();
                         //  unlock all mutexes, since all mutexes were gathered in getMutexWaitTime()
                         for (int i = 0; i < nlists; i++)
                         	pthread_mutex_unlock(&locks[i]);
                         break;
               case SPINLOCK:
			for(int i=0; i <nlists; i++){
				while(__sync_lock_test_and_set(&spinlocks[i],1))
					continue;
			}
                         list_len = getTotalLength();
			for(int i=0; i <nlists; i++){
				 __sync_lock_release(&(spinlocks[i]));
			}
                         break;
                default:
                        break;
       }

      if(list_len < 0){
	 fprintf(stderr, "List is corrupted. \n");
	 exit(2);
      }
    
	for(int k=begin_pos; k < begin_pos + niterations; k++){
    		struct timespec elapsed_start, elapsed_end;
		int sublist_idx = determine_list(element_arr[k].key);
		sublist = &lists[sublist_idx];
		switch(syncOps){
			case DEFAULT:
				checkAndDelete(sublist, k);
				break;
			case MUTEX:	
    				if(clock_gettime(CLOCK_MONOTONIC, &elapsed_start) <0 ){
	 				fprintf(stderr,"Failed to clock_gettime(). Reason: %s. \n", strerror(errno));
					exit(1);
    				}
				pthread_mutex_lock(&locks[sublist_idx]);				
    				if(clock_gettime(CLOCK_MONOTONIC, &elapsed_end) <0 ){
	 				fprintf(stderr,"Failed to clock_gettime(). Reason: %s. \n", strerror(errno));
					exit(1);
    				}
				wait_time[thread_id] += (elapsed_end.tv_sec - elapsed_start.tv_sec) * 1000000000LL + (elapsed_end.tv_nsec - elapsed_start.tv_nsec);
				checkAndDelete(sublist, k);
				pthread_mutex_unlock(&locks[sublist_idx]);
				break;
			case SPINLOCK:
    				if(clock_gettime(CLOCK_MONOTONIC, &elapsed_start) <0 ){
	 				fprintf(stderr,"Failed to clock_gettime(). Reason: %s. \n", strerror(errno));
					exit(1);
    				}
				checkIfSpinLock(sublist_idx);
    				if(clock_gettime(CLOCK_MONOTONIC, &elapsed_end) <0 ){
	 				fprintf(stderr,"Failed to clock_gettime(). Reason: %s. \n", strerror(errno));
					exit(1);
    				}
				wait_time[thread_id] += (elapsed_end.tv_sec - elapsed_start.tv_sec) * 1000000000LL + (elapsed_end.tv_nsec - elapsed_start.tv_nsec);
				checkAndDelete(sublist, k);
				__sync_lock_release(&spinlocks[sublist_idx]);
				break;
			default:
				break;
		}
	}
	return NULL;
}

void signal_handler(int signum){	
	if(signum == SIGSEGV){
		char* error = "Segmentation fault. Exitting with status 2... \n";
		write(STDERR_FILENO, error, 48);
		_exit(2);
	}
}

int main(int argc, char *argv[]) {   
     signal(SIGSEGV, signal_handler);
    //  long options
    struct option long_options[] = {
        {"threads",     required_argument, 0, THREADS},
        {"iterations",  required_argument, 0, ITERATIONS},
        {"sync",        required_argument, 0, SYNC},
        {"yield",       required_argument, 0, YIELD},
	{"lists", 	required_argument, 0, LIST},
        {0,             0,                 0, 0}
    };
    
    int option;
    size_t opt_len;
    
    while ((option = getopt_long(argc, argv, "", long_options, 0)) != -1) {
        switch (option) {
            case THREADS:       nthreads = atoi(optarg);    break;
            case ITERATIONS:    niterations = atoi(optarg); break;
            case YIELD:
                opt_len = strlen(optarg);
                if (opt_len > 3) {
                    fprintf(stderr, "Too many arguments. %lu was passed, but --yield only takes maximum of 3.\n", opt_len);
                    exit(1);
                }
                for (unsigned int i = 0; i < opt_len; i++) {  
                    switch(optarg[i]){
			case 'i': opt_yield |= INSERT_YIELD; break;
			case 'd': opt_yield |= DELETE_YIELD; break;
			case 'l': opt_yield |= LOOKUP_YIELD; break;
			default:
                        	fprintf(stderr, "Invalid parameter for --yield: %c. --yield only takes in 'i', 'd' or 'l'\n", optarg[i]);
                        	exit(1);
                    }
                }
                break;
            case SYNC:
                syncOps = optarg[0];
                if (syncOps != MUTEX && syncOps != SPINLOCK) {
                    fprintf(stderr, "Invalid parameter for --sync: %s. --sync only takes in 's' or 'm'\n", optarg);
                    exit(1);
                }
                break;
	    case LIST:
		nlists=atoi(optarg);
		break;
            default:
                fprintf(stderr, "Invalid option. Usage: ./lab2_list --threads=N --iterations=N --sync'ms' --yield='idl'\n");
                exit(1);
                break;
        }
    }
    
    //Initialize empty list
    lists_locks_init();
    //Create element array
    nelements = nthreads * niterations;
    element_arr = malloc(sizeof(SortedListElement_t) * nelements);
    //Populate the element_arr 
    elem_init();
    if(syncOps == MUTEX || syncOps == SPINLOCK)
	wait_time_init();
     

    //Populate the offset for each thread
    int start_pos_t[nthreads];	
    for(int i =0; i<nthreads; i++)
	start_pos_t[i]= i * niterations;

    //Start time
    struct timespec start_time,end_time;
    if(clock_gettime(CLOCK_MONOTONIC, &start_time) <0 ){
	 fprintf(stderr,"Failed to clock_gettime(). Reason: %s. \n", strerror(errno));
	free_everything(); 
	exit(1);
    }

    pthread_t tids[nthreads];
    
    //Creating threads
    for (int i = 0, start_pos = 0; i < nthreads; i++, start_pos += niterations) {
        start_pos_t[i] = start_pos;
        if (pthread_create(&tids[i], NULL, thread_ops, (void *) &start_pos_t[i])) {
            fprintf(stderr,"Failed to pthread_create(). Reason: %s. \n", strerror(errno));
            free_everything();
	    exit(1);
        }
    }

    //Joining threads
    for (int i = 0; i < nthreads; i++) {
        if (pthread_join(tids[i], NULL)) {
            fprintf(stderr,"Failed to pthread_join(). Reason:%s.\n", strerror(errno));
            free_everything();
	    exit(1);
        }
    }
    //Stop time
    if(clock_gettime(CLOCK_MONOTONIC, &end_time) < 0){
	  fprintf(stderr,"Failed to clock_gettime(). Reason: %s.\n", strerror(errno));
	  free_everything();  
	  exit(1);
    }
    
    //Check length for corruption
    int list_len = 0;
    for(int i=0; i<nlists; i++){
	int sublist_len = SortedList_length(&lists[i]);
	if(sublist_len <0){
		fprintf(stderr,"List is corrupted.\n");
		free_everything();
		exit(2);
	}
	list_len += sublist_len;
    }
    if (list_len != 0) {
        fprintf(stderr, "List is corrupted. Exitting with status 2....\n");
        free_everything();
	exit(2);
    }

    char test[16] = "list";
    switch (opt_yield) {
        case INSERT_YIELD:  strcat(test, "-i");       break;
        case DELETE_YIELD:  strcat(test, "-d");       break;
        case ID_YIELD:      strcat(test, "-id");      break;
        case LOOKUP_YIELD:  strcat(test, "-l");       break;
        case IL_YIELD:      strcat(test, "-il");      break;
        case DL_YIELD:      strcat(test, "-dl");      break;
        case IDL_YIELD:     strcat(test, "-idl");     break;
        default:            strcat(test, "-none");    break;
    }

    if (syncOps == DEFAULT)
        strcat(test, "-none");
    else{	
	char buf[2];
	sprintf(buf,"-%c",syncOps);
	strcat(test,buf);
    }

    long long total_time_elapsed = (end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);
    long long nOps = nthreads * niterations * 3;
    long long avg_time = total_time_elapsed/nOps;
    if(syncOps == MUTEX || syncOps == SPINLOCK){
	long long total_wait_time=0;
	for(int i=0; i<nthreads;i++){
		total_wait_time +=wait_time[i];
	}
	long long avg_wait_time=total_wait_time/nOps;
	printf("%s,%d,%d,%d,%lld,%lld,%lld,%lld\n",test,nthreads,niterations,nlists,nOps,total_time_elapsed, avg_time, avg_wait_time);
    }else{
    	printf("%s,%d,%d,%d,%lld,%lld,%lld\n", test, nthreads, niterations, nlists,
           nOps, total_time_elapsed, avg_time);
    }
    free_everything();
    exit(0);
}

