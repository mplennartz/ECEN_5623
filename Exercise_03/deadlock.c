#include <pthread.h>
#include <stdio.h>
#include <sched.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS  2
#define THREAD_1     1
#define THREAD_2     2

pthread_t threads[ NUM_THREADS ];
struct sched_param nrt_param;

pthread_mutex_t rsrcA, rsrcB;

volatile int rsrcACnt = 0, rsrcBCnt = 0, noWait = 1;

void *grabRsrcs( void *threadid )
{
   if( ( int ) threadid == THREAD_1 )
   {
      printf( "THREAD 1 grabbing resources\n" );
      pthread_mutex_lock( &rsrcA );
      rsrcACnt++;
      usleep( 1000 );
      printf( "THREAD 1 got A, trying for B\n" );
      pthread_mutex_lock( &rsrcB );
      rsrcBCnt++;
      printf( "THREAD 1 got A and B\n" );
      pthread_mutex_unlock( &rsrcB );
      pthread_mutex_unlock( &rsrcA );
      printf( "THREAD 1 done\n" );
   }
   else
   {
      printf( "THREAD 2 grabbing resources\n" );
      pthread_mutex_lock( &rsrcB );
      rsrcBCnt++;
      usleep( 1000 );
      printf( "THREAD 1 got B, trying for A\n" );
      pthread_mutex_lock( &rsrcA );
      rsrcACnt++;
      printf( "THREAD 2 got B and A\n" );
      pthread_mutex_unlock( &rsrcA );
      pthread_mutex_unlock( &rsrcB );
      printf( "THREAD 2 done\n" );
   }

   pthread_exit(NULL);
}

int main( int argc, char *argv[] )
{
   int rc, safe = 0;
   clockid_t clock;
   struct timespec currentTime;
   struct timespec waitTime;

   rsrcACnt = 0, rsrcBCnt = 0, noWait = 1;

   if( argc < 2 )
   {
      printf( "Will set up unsafe deadlock scenario\n" );
   }
   else if( argc == 2 )
   {
      if( strncmp( "safe", argv[ 1 ], 4 ) == 0 )
      {
         safe = 1;
      }
      else if( strncmp( "race", argv[ 1 ], 4 ) == 0 )
      {
         noWait=1;
      }
      else
      {
         printf("Will set up unsafe deadlock scenario\n");
      }
   }
   else
   {
      printf("Usage: deadlock [safe|race|unsafe]\n");
   }

   // Set default protocol for mutex
   pthread_mutex_init( &rsrcA, NULL );
   pthread_mutex_init( &rsrcB, NULL );

   printf( "Creating thread %d\n", THREAD_1 );
   rc = pthread_create( &threads[ 0 ], NULL, grabRsrcs, ( void * ) THREAD_1 );

   if( rc )
   {
      printf( "ERROR; pthread_create() rc is %d\n", rc );
      perror( NULL );
      exit( -1 );
   }

   printf("Thread 1 spawned\n");

   if( safe ) // Make sure Thread 1 finishes with both resources first
   {
      if( pthread_join( threads[ 0 ], NULL ) == 0 )
      {
         printf( "Thread 1: %d done\n", threads[ 0 ] );
      }
      else
      {
         perror( "Thread 1" );
      }
   }

   printf( "Creating thread %d\n", THREAD_2 );
   rc = pthread_create( &threads[ 1 ], NULL, grabRsrcs, ( void * ) THREAD_2 );

   if( rc )
   {
      printf( "ERROR; pthread_create() rc is %d\n", rc );
      perror( NULL );
      exit( -1 );
   }

   printf( "Thread 2 spawned\n" );
   printf( "rsrcACnt = %d, rsrcBCnt = %d\n", rsrcACnt, rsrcBCnt );
   printf( "will try to join CS threads unless they deadlock\n" );

   if( !safe )
   {
      clock_gettime( clock, &currentTime );
      srand( ( unsigned ) currentTime.tv_nsec );

      int completionStatus;
      int notDone = 1;
      while( notDone )
      {
         usleep( 2000 );
         waitTime.tv_sec = 0;
         waitTime.tv_nsec = rand( );
         printf( "Wait time = %dns\n", waitTime.tv_nsec );
         completionStatus = pthread_timedjoin_np( threads[ 0 ], NULL, &waitTime );

         if( completionStatus )
         {
            printf( "Destroying thread 1 and restarting it\n" );
            pthread_cancel( threads[ 0 ] );
            pthread_mutex_unlock( &rsrcB );
            pthread_mutex_unlock( &rsrcA );
            rc = pthread_create( &threads[ 0 ], NULL, grabRsrcs, ( void * ) THREAD_1 );

            if( rc )
            {
               printf( "ERROR; pthread_create() rc is %d\n", rc );
               perror( NULL );
               exit( -1 );
            }

            sleep( 1 );
         }
         else
         {
            notDone = 0;
         }
      }
   }

   if( pthread_join( threads[ 1 ], NULL ) == 0 )
   {
      printf("Thread 2: %d done\n", threads[ 1 ] );
   }
   else
   {
      perror( "Thread 2" );
   }

   if( pthread_mutex_destroy( &rsrcA ) != 0 )
   {
      perror( "mutex A destroy" );
   }

   if( pthread_mutex_destroy( &rsrcB ) != 0 )
   {
      perror( "mutex B destroy" );
   }

   printf( "All done\n" );

   exit( 0 );
}
