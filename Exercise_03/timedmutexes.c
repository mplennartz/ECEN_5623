//All required headers, function definitions, 
//  global variables moved to mutexex.h
#include "mutexes.h"

void UpdateAttitude( );
void ReadAttitude( );

static void ( *ThreadFunctions[ NUM_THREADS ] ) =
              { UpdateAttitude,
                ReadAttitude };

int main( int argc, char** argv )
{
   pthread_t threads[ NUM_THREADS ];
   int thread_args[ NUM_THREADS ];
   int result_code;
   unsigned index;
   char log[10] = "[ MAIN ]";

   //Only require valid = 0
   //if valid = 0, data assumed not valid
   attitude.valid		 = 0;

   // create all threads one by one
   for( index = 0; index < NUM_THREADS; ++index )
   {
      thread_args[ index ] = index;
      printf("%s creating thread %d\n", log_time(log),index);
      result_code = pthread_create( &threads[ index ],
                                    NULL,
                                    ThreadFunctions[ index ],
                                    &thread_args[index] );
      assert( !result_code );
   }

   // wait for each thread to complete
   for( index = 0; index < NUM_THREADS; ++index )
   {
      // block until thread 'index' completes
      result_code = pthread_join( threads[ index ], NULL );
      assert( !result_code );
      printf( "%s thread %d has completed\n", log_time(log),index );
   }

   printf( "%s All threads completed successfully\n",log_time(log) );
   exit( EXIT_SUCCESS );
}

void UpdateAttitude( void* argument )
{
   char log[10] = "[UPDATE]";
   while( 1 )
   {
      printf( "%s Waiting for lock.\n",log_time(log) );
      // Locking the access to the attitude structure
      pthread_mutex_lock( &attitudeMut );
      printf( "%s Lock obtained.\n",log_time(log) );
      printf( "%s Attitude data is being updated\n",log_time(log) );
      //Discussed randomizing sleep value between 5 to 15 seconds 
      //to show vary updating time but decided to leave to
      //always show the timelock timeout
      sleep( 14 );   
      // The sleep will help show that the mutex is working.
      // Thread 2 that reads is running is waiting for the mutex to unlock
      // If the mutex is not used, then the ReadAttitude thread always reads 0's
      // Getting the time when the data is updated and is used to create random numbers.
      clock_gettime( CLOCK_REALTIME, &attitude.sampleTime );
      srand( ( unsigned ) attitude.sampleTime.tv_nsec );

      attitude.x            = rand( ) + ( ( double ) rand( ) / ( double ) RAND_MAX );
      attitude.y            = rand( ) + ( ( double ) rand( ) / ( double ) RAND_MAX );
      attitude.z            = rand( ) + ( ( double ) rand( ) / ( double ) RAND_MAX );
      attitude.acceleration = rand( ) + ( ( double ) rand( ) / ( double ) RAND_MAX );
      attitude.roll         = rand( ) + ( ( double ) rand( ) / ( double ) RAND_MAX );
      attitude.pitch        = rand( ) + ( ( double ) rand( ) / ( double ) RAND_MAX );
      attitude.yaw          = rand( ) + ( ( double ) rand( ) / ( double ) RAND_MAX );
      attitude.valid = 1;
      printf( "%s Update complete.\n",log_time(log) );
      printf( "%s Releasing lock.\n",log_time(log) );
      pthread_mutex_unlock( &attitudeMut );
      sleep( 1 );
   }
}

void ReadAttitude( )
{
   char log[10] = "[READER]";
   struct timespec currentTimeSpec;
   struct tm *currentTime;
   struct tm *attitude_timestamp;
   struct timespec timeout;
   timeout.tv_sec = 10;
   timeout.tv_nsec = 0;

   while( 1 )
   {
      printf( "%s Waiting for lock.\n",log_time(log) );
      clock_gettime( CLOCK_REALTIME, &timeout );
      timeout.tv_sec += 10;
      if( pthread_mutex_timedlock( &attitudeMut, &timeout ) != 0 ) {
         clock_gettime( CLOCK_REALTIME, &currentTimeSpec );
         currentTime = localtime( &currentTimeSpec );
         printf( "%s No new data at time %02d:%02d:%02d\n", log_time(log),
                  currentTime->tm_hour, currentTime->tm_min, currentTime->tm_sec );

      }else{
         printf( "%s Lock obtained.\n",log_time(log) );
         if( attitude.valid ) {
            attitude_timestamp = localtime( &attitude.sampleTime );
            printf( "%s Data valid\n",log_time(log) );
            printf( "%s Reading Attitude data\n",log_time(log) );
            printf( "%s Attitude data:\n"
                     "\t\t\t\ttimestamp    = %02d:%02d:%02d.%lu\n"
                     "\t\t\t\tx            = %lf\n"
                     "\t\t\t\ty            = %lf\n"
                     "\t\t\t\tz            = %lf\n"
                     "\t\t\t\tacceleration = %lf\n"
                     "\t\t\t\troll         = %lf\n"
                     "\t\t\t\tpitch        = %lf\n"
                     "\t\t\t\tyaw          = %lf\n",
                     log_time(log),
                     attitude_timestamp->tm_hour,
                     attitude_timestamp->tm_min,
                     attitude_timestamp->tm_sec,
                     attitude.sampleTime.tv_nsec,
                     attitude.x,
                     attitude.y,
                     attitude.z,
                     attitude.acceleration,
                     attitude.roll,
                     attitude.pitch,
                     attitude.yaw 
                  );
         }else{
            printf( "%s Data not valid\n",log_time(log) );
         }
         printf( "%s Releasing lock.\n",log_time(log) );
         pthread_mutex_unlock( &attitudeMut );
         sleep( 1 );
      }
   }
}
