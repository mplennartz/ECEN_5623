/****************************************************************************/
/*                                                                          */
/* Sam Siewert - 10/14/97                                                   */
/*                                                                          */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <mqueue.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <stdint.h>

#define SNDRCV_MQ       "/send_receive_mq"
#define ERROR           -1
#define NUM_THREADS     2

void shutdown(void);
void receiver(void);
void sender(void);

struct mq_attr mq_attr;
static mqd_t mymq;
static char imagebuff[4096];
static int sid, rid;


static void ( *ThreadFunctions[ NUM_THREADS ] ) =
             { sender,
               receiver };

void main(void)
{
   int i, j;
   char pixel = 'A';
   pthread_t threads[ NUM_THREADS ];
   int thread_args[ NUM_THREADS ];
   int result_code;
   unsigned index;

   for( i = 0; i < 4096; i += 64 )
   {
      pixel = 'A';

      for( j = i; j < i + 64; j++ )
      {
         imagebuff[ j ] = ( char ) pixel++;
      }

      imagebuff[ j-1 ] = '\n';
   }

   imagebuff[ 4095 ] = '\0';
   imagebuff[ 63 ] = '\0';

   /* setup common message q attributes */
   mq_attr.mq_maxmsg = 100;
   mq_attr.mq_msgsize = sizeof( void * ) + sizeof( int );
   mq_attr.mq_flags = 0;

   /* note that VxWorks does not deal with permissions? */
   mymq = mq_open( SNDRCV_MQ, O_CREAT | O_RDWR, 0, &mq_attr );

   if( mymq == ( mqd_t ) ERROR )
   {
      perror( "mq_open" );
   }


   // Create two communicating processes right here
   for( index = 0; index < NUM_THREADS; ++index )
   {
      thread_args[ index ] = index;
      printf("In main: creating thread %d\n", index);
      result_code = pthread_create( &threads[ index ],
                                    NULL,
                                    ThreadFunctions[ index ],
                                    &thread_args[index] );
   }

   // wait for each thread to complete
   for( index = 0; index < NUM_THREADS; ++index )
   {
      // block until thread 'index' completes
      result_code = pthread_join( threads[ index ], NULL );

      printf( "In main: thread %d has completed\n", index );
   }

   printf( "In main: All threads completed successfully\n" );
   mq_close(mymq);
}



/* receives pointer to heap, reads it, and deallocate heap memory */

void receiver( void )
{
   char buffer[ sizeof( void * ) + sizeof( int ) ];
   void *buffptr;
   int prio;
   int nbytes;
   int count = 0;
   int id;

   while( 1 )
   {
      /* read oldest, highest priority msg from the message queue */

      printf( "Reading %ld bytes\n", sizeof( void * ) );

      if( ( nbytes = mq_receive( mymq, buffer, ( size_t )( sizeof( void * ) + sizeof( int ) ), &prio ) ) == ERROR )
      {
         perror( "mq_receive" );
      }
      else
      {
         memcpy( &buffptr, buffer, sizeof( void * ) );
         memcpy( ( void * )&id, &( buffer[ sizeof( void * ) ] ), sizeof( int ) );
         printf( "receive: ptr msg 0x%X received with priority = %d, length = %d, id = %d\n", buffptr, prio, nbytes, id );
         printf( "receive: contents of ptr = \n%s\n", ( char * ) buffptr );

         free( buffptr );

         printf( "receive: heap space memory freed\n" );
      }
   }
}

void sender( void )
{
   char buffer[ sizeof( void * ) + sizeof( int ) ];
   void *buffptr;
   int prio;
   int nbytes;
   int id = 999;

   while( 1 )
   {
      /* send malloc'd message with priority=30 */

      buffptr = ( void * ) malloc( sizeof( imagebuff ) );
      strcpy( buffptr, imagebuff );
      printf( "Message to send = %s\n", (char *)buffptr );

      printf( "Sending %ld bytes\n", sizeof( buffptr ) );

      memcpy( buffer, &buffptr, sizeof( void * ) );
      memcpy( &( buffer[ sizeof( void * ) ] ), ( void * )&id, sizeof( int ) );

      if( ( nbytes = mq_send( mymq, buffer, ( size_t )( sizeof( void * ) + sizeof( int ) ), 30 ) ) == ERROR )
      {
         perror( "mq_send" );
      }
      else
      {
         printf( "send: message ptr 0x%X successfully sent\n", buffptr );
      }

      sleep( 3 );

   }

}
