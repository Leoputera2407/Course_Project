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
static int spinlock             = 0;
static char syncOps             = DEFAULT;

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static SortedList_t list;
static SortedListElement_t *element_arr;

void list_init(){	
    SortedList_t* head = &list;
    list.key = NULL;
    list.next = head;
    list.prev = head;
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

void checkAndDelete(int position){
	SortedListElement_t *elem = SortedList_lookup(&list, element_arr[position].key);
	if(elem == NULL || SortedList_delete(elem) != 0){
		fprintf(stderr,"List is corrupted. \n");
		exit(2);
	}
}	

void* thread_ops(void* start_pos){
	int begin_pos = *((int*) start_pos);
	for(int j=begin_pos; j < begin_pos + niterations; j++){
		switch(syncOps){
			case DEFAULT:
				SortedList_insert(&list,&element_arr[j]);
				break;
			case MUTEX:
				pthread_mutex_lock(&lock);
				SortedList_insert(&list, &element_arr[j]);
				pthread_mutex_unlock(&lock);
				break;
			case SPINLOCK:
				while(__sync_lock_test_and_set(&spinlock,1))
					continue;
				SortedList_insert(&list, &element_arr[j]);
				__sync_lock_release(&spinlock);
				break;
			default:
				break;
		}
	}
	int list_len=0;
	switch(syncOps){
        	case DEFAULT:
              		list_len = SortedList_length(&list);
                       	 break;
                case MUTEX:
                         pthread_mutex_lock(&lock);
                         list_len = SortedList_length(&list);
                         pthread_mutex_unlock(&lock);
                         break;
               case SPINLOCK:
                       	 while(__sync_lock_test_and_set(&spinlock,1))
                         	continue;
                         list_len = SortedList_length(&list);
                         __sync_lock_release(&spinlock);
                         break;
                default:
                        break;
       }

      if(list_len < 0){
	 fprintf(stderr, "List is corrupted. \n");
	 exit(2);
      }
    
	for(int k=begin_pos; k < begin_pos + niterations; k++){
		switch(syncOps){
			case DEFAULT:
				checkAndDelete(k);
				break;
			case MUTEX:
				pthread_mutex_lock(&lock);
				checkAndDelete(k);
				pthread_mutex_unlock(&lock);
				break;
			case SPINLOCK:
				while(__sync_lock_test_and_set(&spinlock,1))
					continue;
				checkAndDelete(k);
				__sync_lock_release(&spinlock);
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
            default:
                fprintf(stderr, "Invalid option. Usage: ./lab2_list --threads=N --iterations=N --sync'ms' --yield='idl'\n");
                exit(1);
                break;
        }
    }
    
    //Initialize empty list
    list_init();
    //Create element array
    nelements = nthreads * niterations;
    element_arr = malloc(sizeof(SortedListElement_t) * nelements);
    //Populate the element_arr 
    elem_init();

    //Populate the offset for each thread
    int start_pos_t[nthreads];	
    for(int i =0; i<nthreads; i++)
	start_pos_t[i]= i * niterations;

    //Start time
    struct timespec start_time,end_time;
    if(clock_gettime(CLOCK_MONOTONIC, &start_time) <0 ){
	 fprintf(stderr,"Failed to clock_gettime(). Reason: %s. \n", strerror(errno));
	 free(element_arr);
	 exit(1);
    }

    pthread_t tids[nthreads];
    
    //Creating threads
    for (int i = 0, start_pos = 0; i < nthreads; i++, start_pos += niterations) {
        start_pos_t[i] = start_pos;
        if (pthread_create(&tids[i], NULL, thread_ops, (void *) &start_pos_t[i])) {
            fprintf(stderr,"Failed to pthread_create(). Reason: %s. \n", strerror(errno));
            free(element_arr);
            exit(1);
        }
    }

    //Joining threads
    for (int i = 0; i < nthreads; i++) {
        if (pthread_join(tids[i], NULL)) {
            fprintf(stderr,"Failed to pthread_join(). Reason:%s.\n", strerror(errno));
            free(element_arr);
            exit(1);
        }
    }
    //Stop time
    if(clock_gettime(CLOCK_MONOTONIC, &end_time) < 0){
	  fprintf(stderr,"Failed to clock_gettime(). Reason: %s.\n", strerror(errno));
	  free(element_arr);
	  exit(1);
    }
    
    //Check length for corruption
    int list_len = SortedList_length(&list);
    if (list_len != 0) {
        fprintf(stderr, "List is corrupted. Exitting with status 2....\n");
        free(element_arr);
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

    long long total_time_elapsed = (end_time.tv_sec - start_time.tv_sec) * 1000000000 + (end_time.tv_nsec - start_time.tv_nsec);
    long long nOps = nthreads * niterations * 3;
    long long avg_time = total_time_elapsed/nOps;
    printf("%s,%d,%d,%d,%lld,%lld,%lld\n", test, nthreads, niterations, nlists,
           nOps, total_time_elapsed, avg_time);
    free(element_arr);
    exit(0);
}
