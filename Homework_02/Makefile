#*****************************************************
#Makefile
#
#Author: 	M. Lennartz
#Revision: 1.0
#Purpose:
#The makefile just simplifies the compiling and
#running process. The use calls are below:
#
#make all - will clean the directory of all 
#executables and then recompile all executable
#processes
#
#make clean - remove all executables
#
#make run - will run the run.sh script to run the 
#p_sem_wait and p_sem_post processes
#*****************************************************/
all: clean p_sem_wait p_sem_post p_sem_clean

p_sem_wait:
	gcc process.h p_sem_wait.c -o p_sem_wait -lpthread

p_sem_post:
	gcc process.h p_sem_post.c -o p_sem_post -lpthread

p_sem_clean:
	gcc process.h p_sem_clean.c -o p_sem_clean -lpthread
   
clean:
	rm -f p_sem_wait
	rm -f p_sem_post
	rm -f p_sem_clean

run:
	./run.sh
