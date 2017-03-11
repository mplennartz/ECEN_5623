/* 
 * Author: Nisheeth 	Created on: Apr 4, 2011
 * Update: Lennartz		Updated on: Feb 6, 2017
 *
 *   Updated using methodologies used in prior assignements.
 * Removed custom functions for calculated time and used
 * clock_gettime functions and the previously defined 
 * delta_t function.
 */
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <unistd.h>
#include <time.h>
#include <syslog.h>
#include <math.h>
#include <sys/param.h>


#define FIB_LIMIT_FOR_32_BIT 10
#define REQ_INTER 2000000
#define TIME_10_MSEC 10000

#define NSEC_PER_SEC (1000000000)
#define NSEC_PER_MSEC (1000000)
#define NSEC_PER_MICROSEC (1000)

/********************************************************************
 Variable Declarations
********************************************************************/
pthread_t 		testThread10;
pthread_t 		testThread20;
pthread_attr_t 	rt10_sched_attr;
pthread_attr_t 	rt20_sched_attr;
pthread_attr_t 	main_sched_attr;
sem_t 			sem_t10;
sem_t			sem_t20;
int 			rt_max_prio;
int				rt_min_prio;
int				min;
double 			start_1       = 0;
int				abortTest_10  = 0;
int				abortTest_20  = 0;
int 			seqIterations = FIB_LIMIT_FOR_32_BIT;

struct timespec start_time = {0, 0};

struct sched_param rt10_param;
struct sched_param rt20_param;
struct sched_param nrt_param;
struct sched_param main_param;
/********************************************************************
 Fibonaci Delay
********************************************************************/
#define FIB_TEST(seqCnt, iterCnt)      \
   for(idx=0; idx < iterCnt; idx++)    \
   {                                   \
      fib = fib0 + fib1;               \
      while(jdx < seqCnt)              \
      {                                \
         fib0 = fib1;                  \
         fib1 = fib;                   \
         fib = fib0 + fib1;            \
         jdx++;                        \
      }                                \
   }                                   \

/********************************************************************
 DELTA_T Function Definitions
********************************************************************/
int delta_t(struct timespec *stop, struct timespec *start, struct timespec *delta_t)
{
  int dt_sec=stop->tv_sec - start->tv_sec;
  int dt_nsec=stop->tv_nsec - start->tv_nsec;

  if(dt_sec >= 0){
    if(dt_nsec >= 0){
      delta_t->tv_sec=dt_sec;
      delta_t->tv_nsec=dt_nsec;
    }else{
      delta_t->tv_sec=dt_sec-1;
      delta_t->tv_nsec=NSEC_PER_SEC+dt_nsec;
    }
  }else{
    if(dt_nsec >= 0){
      delta_t->tv_sec=dt_sec;
      delta_t->tv_nsec=dt_nsec;
    }else{
      delta_t->tv_sec=dt_sec-1;
      delta_t->tv_nsec=NSEC_PER_SEC+dt_nsec;
    }
  }

  return(1);
}
/********************************************************************
 Simply prints the current POSIX scheduling policy in effect.
********************************************************************/
void print_scheduler(void)
{
	int schedType;
	schedType = sched_getscheduler(getpid());

	switch(schedType){
		case SCHED_FIFO:
			printf("Pthread Policy is SCHED_FIFO\n");
			break;
		case SCHED_OTHER:
			printf("Pthread Policy is SCHED_OTHER\n");
			break;
		case SCHED_RR:
			printf("Pthread Policy is SCHED_OTHER\n");
			break;
		default:
			printf("Pthread Policy is UNKNOWN\n");
	}
}

/********************************************************************
 Thread10
********************************************************************/
void *Thread10(void *threadid){
	struct sched_param param;
    struct timespec finish_time = {0, 0};
    struct timespec thread_dt   = {0, 0};
	int policy;
	int idx = 0, jdx = 1, fib = 0, fib0 = 0, fib1 = 1;
	unsigned long mask = 1; /* processor 0 */


	while (!abortTest_10){
		sem_wait (& sem_t10 );
		FIB_TEST(seqIterations, REQ_INTER);
		clock_gettime(CLOCK_REALTIME, &finish_time);
		pthread_getschedparam(testThread10,&policy ,&param);
		delta_t(&finish_time, &start_time, &thread_dt);
		printf("Thread10 priority = %d and time stamp %lf msec\n",param.sched_priority,((double)thread_dt.tv_nsec /(double)NSEC_PER_MSEC));
	}

}

/********************************************************************
 Thread20
********************************************************************/
void *Thread20(void *threadid){
	struct sched_param param;
    struct timespec finish_time = {0, 0};
    struct timespec thread_dt   = {0, 0};
	int policy;
	int idx = 0, jdx = 1, fib = 0, fib0 = 0, fib1 = 1;
	unsigned long mask = 1; /* processor 0 */

	while (!abortTest_20){
		sem_wait (& sem_t20 );
		FIB_TEST(seqIterations*3, REQ_INTER*3);
		clock_gettime(CLOCK_REALTIME, &finish_time);
		pthread_getschedparam(testThread20,&policy ,&param);
		delta_t(&finish_time, &start_time, &thread_dt);
		printf("Thread20 priority = %d and time stamp %lf msec\n",param.sched_priority,((double)thread_dt.tv_nsec /(double)NSEC_PER_MSEC));
	}
}

/********************************************************************
 Main
********************************************************************/
int main (int argc, char *argv[]){
	int 		rc;
	int			scope,i;
	double 		stop_1 = 0;

    struct timespec finish_time = {0, 0};
    struct timespec thread_dt = {0, 0};

	useconds_t 	t_10 = TIME_10_MSEC;
	useconds_t	t_20 = TIME_10_MSEC*2;

	sem_init (& sem_t10, 0, 1 );
	sem_init (& sem_t20, 0, 1 );

	printf("Before adjustments to scheduling policy:\n");
	print_scheduler();

	pthread_attr_init(&rt10_sched_attr);
	pthread_attr_init(&rt20_sched_attr);
	pthread_attr_init(&main_sched_attr);

	pthread_attr_setinheritsched(&rt10_sched_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setinheritsched(&rt20_sched_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setinheritsched(&main_sched_attr, PTHREAD_EXPLICIT_SCHED);

	pthread_attr_setschedpolicy(&rt10_sched_attr, SCHED_FIFO);
	pthread_attr_setschedpolicy(&rt20_sched_attr, SCHED_FIFO);
	pthread_attr_setschedpolicy(&main_sched_attr, SCHED_FIFO);

	rt_max_prio = sched_get_priority_max(SCHED_FIFO);
	rt_min_prio = sched_get_priority_min(SCHED_FIFO);

	rc=sched_getparam(getpid(), &nrt_param);

	//Large priority = Highest priority
	main_param.sched_priority = rt_max_prio;		// set 99
	rt10_param.sched_priority = rt_max_prio-1;		// set 98
	rt20_param.sched_priority = rt_max_prio-2;		// set 97

	if (sched_setscheduler(getpid(), SCHED_FIFO, &main_param)){
		printf("ERROR; sched_setscheduler rc is %d\n", rc); perror(NULL); exit(-1);
	}

	printf("After adjustments to scheduling policy:\n");
	print_scheduler();
	printf("min prio = %d, max prio = %d\n", rt_min_prio, rt_max_prio);
	pthread_attr_getscope(&rt10_sched_attr, &scope);

	pthread_attr_setschedparam(&rt10_sched_attr, &rt10_param);
	pthread_attr_setschedparam(&rt20_sched_attr, &rt20_param);
	pthread_attr_setschedparam(&main_sched_attr, &main_param);

	clock_gettime(CLOCK_REALTIME, &start_time);

	if (pthread_create (& testThread10, & rt10_sched_attr, Thread10 , ( void *)0 )){ 	//Error if not created
		printf("ERROR during pthread_create() for Thread10\n"); 
		perror(NULL); 
		exit(-1);
	}
	sem_wait (& sem_t10 );	

	if (pthread_create (& testThread20, & rt20_sched_attr, Thread20 , ( void *)0 )){	//Error if not created
		printf("ERROR during pthread_create() for Thread20\n"); 
		perror(NULL); 
		exit(-1);
	}
	sem_wait (& sem_t20 );	

	/* Basic sequence of releases after CI */
	sem_post (& sem_t10 );
	sem_post (& sem_t20 );
	usleep( t_20 );
	sem_post (& sem_t10 );
	usleep( t_20 );
	sem_post (& sem_t10 );
	usleep( t_10 );
	abortTest_20 = 1;
	sem_post (& sem_t20 );
	usleep( t_10 );
	sem_post (& sem_t10 );
	usleep( t_20 );
	abortTest_10 = 1;
	sem_post (& sem_t10 );
	usleep( t_20 );

	clock_gettime(CLOCK_REALTIME, &finish_time);
	delta_t(&finish_time, &start_time, &thread_dt);

	printf("Test Conducted over %lf msec\n",((double)thread_dt.tv_nsec /(double)NSEC_PER_MSEC));
	pthread_join( testThread10, NULL );
	pthread_join( testThread20, NULL );
	if(pthread_attr_destroy(&rt10_sched_attr) != 0)
	perror("attr destroy");
	if(pthread_attr_destroy(&rt20_sched_attr) != 0)
	perror("attr destroy");
	sem_destroy(&sem_t10);
	sem_destroy(&sem_t20);
	rc=sched_setscheduler(getpid(), SCHED_OTHER, &nrt_param);
	printf("TEST COMPLETE\n");

}
