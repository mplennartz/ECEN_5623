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
   char log [10] = "[ MAIN ]";

   //Only require valid = 0
   //if valid = 0, data assumed not valid
   attitude.valid		 = 0;

   // create all threads one by one
   for( index = 0; index < NUM_THREADS; ++index )
   {
      thread_args[ index ] = index;
      printf( "%s Creating thread %d\n", log_time(log) ,index );
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
      printf( "%s Thread %d has completed\n",log_time(log) ,index );
   }

   printf( "%s All threads completed successfully\n",log_time(log) );
   exit( EXIT_SUCCESS );
}

void UpdateAttitude( void* argument )
{
   char log[10] = "[UPDATE]";
   clockid_t clock;
   // Locking the access to the attitude structure
   printf( "%s Waiting for lock.\n",log_time(log) );
   pthread_mutex_lock( &attitudeMut );
   printf( "%s Lock obtained.\n",log_time(log) );
   printf( "%s Attitude data is being updated\n",log_time(log) );
   // The sleep will help show that the mutex is working.
   // Thread 2 that reads is running is waiting for the mutex to unlock
   // If the mutex is not used, then the ReadAttitude thread always reads 0's
   sleep( 1 );
   // Getting the time when the data is updated and is used to create random numbers.
   clock_gettime( clock, &attitude.sampleTime );
   srand( ( unsigned ) attitude.sampleTime.tv_nsec );

   attitude.x            = rand( ) + ( ( double ) rand( ) / ( double ) RAND_MAX );
   attitude.y            = rand( ) + ( ( double ) rand( ) / ( double ) RAND_MAX );
   attitude.z            = rand( ) + ( ( double ) rand( ) / ( double ) RAND_MAX );
   attitude.acceleration = rand( ) + ( ( double ) rand( ) / ( double ) RAND_MAX );
   attitude.roll         = rand( ) + ( ( double ) rand( ) / ( double ) RAND_MAX );
   attitude.pitch        = rand( ) + ( ( double ) rand( ) / ( double ) RAND_MAX );
   attitude.yaw          = rand( ) + ( ( double ) rand( ) / ( double ) RAND_MAX );
   attitude.valid = 1;  //Data now valid for reading
   printf( "%s Update complete.\n",log_time(log) );
   printf( "%s Releasing lock.\n",log_time(log) );
   pthread_mutex_unlock( &attitudeMut );
   sleep( 1 );
}

void ReadAttitude( )
{
   char log[10] = "[READER]";
   int  read_complete = 0;
   struct tm *attitude_timestamp;

   while(read_complete == 0){
      printf( "%s Waiting for lock.\n",log_time(log) );
      pthread_mutex_lock( &attitudeMut );
      printf( "%s Lock obtained.\n",log_time(log) );
      sleep(1); //Allows for the other thread to show it is being blocked
      //Checks if data in struct is valid
      if(attitude.valid){
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
         read_complete = 1;
         printf( "%s Read complete.\n",log_time(log) );
      }else{
         printf( "%s Data not valid\n",log_time(log) );
      }
      printf( "%s Releasing lock.\n",log_time(log) );
      pthread_mutex_unlock( &attitudeMut );
      sleep(1);
   }
}
