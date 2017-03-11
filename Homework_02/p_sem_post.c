/*****************************************************
p_sem_post.c

Author: 	M. Lennartz
Revision: 1.0
Purpose:
This process is used with p_sem_wait process. This
process can be called after running p_sem_wait to 
post to the named semaphore and unblocking p_sem_wait
process.  This process will then run a delay loop
before finishing.
*****************************************************/
//All needed headers, defines, and function
//definitions are located in process.h
#include "process.h"

int main(int argc, char *argv[]){
  unsigned int i,j;

  //Open named semaphore
  sem_t *sem = sem_open( _SEM, 0);

  p2s(PR2,"Post to ""Named"" semaphore.");
	//"Post" to semaphore, unblocking.
  sem_post( sem );

  // General delay x4
  for(i=0; i< 1000000; i++){
    for(j=0; j<400; j++){}
  }
  p2s(PR2,"Finished delay and exiting.");

  return 0;
}
