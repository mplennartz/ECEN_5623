#!/bin/bash
#*****************************************************
#run.sh
#
#Author: 	M. Lennartz
#Revision: 1.0
#Purpose:
#This script is just to automate the sequence of 
#running the processes and run the ps command.  The
#sleep commands are used to give some time to print
#the running processes to the screen.  With a delay
#of several seconds, a user can actually see the time
#delay on the screen
#*****************************************************/
printf "SHELL : Starting process p_sem_wait\n"
#Run p_sem_wait in the background
./p_sem_wait &
sleep 5s
printf "\nSHELL : Currently running processes\n"
#List running processes on screen
ps
sleep 1s
printf "\nSHELL : Starting process p_sem_post\n"
#Run p_sem_post in foreground and exit when complete
./p_sem_post
