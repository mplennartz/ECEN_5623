/*****************************************************
p_sem_wait.c

Author: 	M. Lennartz
Revision: 1.0
Purpose:
This process is used with p_sem_post process. This 
process is run first, creating a named semaphore and
advancing until the sem_wait call.  The semaphore is
created with an initial value of 0, requiring a "post"
to advance.  After the p_sem_post processes posts to
the semaphore, this process can continue.  This
process will run through a delay before unlinking the
named semaphore and finishing.
*****************************************************/
//All needed headers, defines, and function
//definitions are located in process.h
#include "process.h"

int main(int argc, char *argv[]){
  unsigned int i,j;
  // Create a named semaphore, 
  sem_t *sem = sem_open( _SEM, O_CREAT, 0644, 0);
  p2s(PR1,"Finished creating ""Named"" semaphore.");

  p2s(PR1,"Waiting for semaphore post.");
	//Wait for semaphore to be posted/incremented
	//when named semaphore is a non-zero value.
	//When the value is non-zero, this process will
	//increment and continue to run.
  sem_wait( sem );
  p2s(PR1,"Semaphore non-zero, continuing.");

  // General delay x1
  for(i=0; i< 1000000; i++){
    for(j=0; j<100; j++){}
  }

  p2s(PR1,"Finished delay.");
	//"Unlink" named semaphore
	//releasing associated memory 
  sem_unlink( _SEM );
  p2s(PR1,"Unlinked ""Named"" semaphore and exiting.");

  return 0;
}
