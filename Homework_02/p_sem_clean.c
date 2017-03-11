/*****************************************************
p_sem_clean.c

Author: 	M. Lennartz
Revision: 1.0
Purpose:
Used for debugging p_sem_post and p_sem_wait
processes.  In the event an issue is created, this
process can be run to unline the named semaphore
created by p_sem_wait process.
*****************************************************/
//All needed headers, defines, and function
//definitions are located in process.h
#include "process.h"

int main(int argc, char *argv[]){
	//Open named semaphore
  sem_t *sem = sem_open( _SEM, 0);

  p2s(PR3,"Clearing semaphore.");
	//"Post" to semaphore
  sem_post( sem );
  p2s(PR3,"Unlinking named semaphore.");
	//"Unlink" named semaphore
	//releasing associated memory 
  sem_unlink( _SEM );
  p2s(PR3,"Exit.");

  return 0;
}
