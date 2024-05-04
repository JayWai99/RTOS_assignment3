/******************************************************************************* 
The assignment 3 for subject 48450 (RTOS) in University of Technology Sydney(UTS) 
This is a template of Program_1.c template. Please complete the code based on 
the assignment 3 requirement. Assignment 3 

------------------------------Program_1.c template------------------------------
*******************************************************************************/

#include <pthread.h> 	/* pthread functions and data structures for pipe */
#include <unistd.h> 	/* for POSIX API */
#include <stdlib.h> 	/* for exit() function */
#include <stdio.h>	/* standard I/O routines */
#include <stdbool.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>

#define PROCESS_NUMBER 7

typedef struct RR_Params {
  //add your variables here
  int process_id;
  int arrive_time, wait_time, burst_time, turn_around_time, remain_time;
} ThreadParams;

ThreadParams processes[8];

sem_t sem_RR;

int i;

pthread_t thread_A, thread_B;

void set_input_processes() {
	processes[0].process_id = 1; processes[0].arrive_time = 8; processes[0].burst_time = 10;
	processes[1].process_id = 2; processes[1].arrive_time = 10; processes[1].burst_time = 3;
	processes[2].process_id = 3; processes[2].arrive_time = 14; processes[2].burst_time = 7;
	processes[3].process_id = 4; processes[3].arrive_time = 9; processes[3].burst_time = 5;
	processes[4].process_id = 5; processes[4].arrive_time = 16; processes[4].burst_time = 4;
	processes[5].process_id = 6; processes[5].arrive_time = 21; processes[5].burst_time = 6;
	processes[6].process_id = 7; processes[6].arrive_time = 26; processes[6].burst_time = 2;

     	//remain_time = burst_time at this point because nothing has
		// been processed yet.
	for (i = 0; i < PROCESS_NUMBER; i++) {
		processes[i].remain_time = processes[i].burst_time;
	}
}

/* this function calculates Round Robin (RR) with a time quantum of 4, writes waiting time and turn-around time to the RR */
void *worker1(void *params)
{
   // add your code here
   set_input_processes();
   //run_process_RR();
   //calculate_average();
   //fifo_write();
   sem_post(&sem_RR);
}

/* reads the waiting time and turn-around time through the RR and writes to text file */
void *worker2()
{
   // add your code here
   sem_wait(&sem_RR);
   //fifo_read();
   //write_file();
}

/* this main function creates named pipe and threads */
int main(void)
{
	/* creating a named pipe(RR) with read/write permission */
	// add your code 

	/* initialize the parameters */
	 // add your code 
	if (sem_init(&sem_RR, 0, 0) != 0){
		perror("Failed to initialise semaphore.");
		exit(-1);
	}

	/* create threads */
	 // add your code
	if (pthread_create(&thread_A, NULL, worker1, NULL)){
		perror("Failed to create thread.");
		exit(-2);
	}
	
	if (pthread_create(&thread_B, NULL, worker2, NULL)){
		perror("Failed to create thread.");
		exit(-2);
	}

	/* wait for the thread to exit */
	//add your code
	if(pthread_join(thread_A, NULL)!=0)
	{
	    perror("Failed to join thread.");
		exit(-3);
	}
	if(pthread_join(thread_B, NULL)!=0)
	{
	    perror("Failed to join thread.");
		exit(-3);
	}
	
	if(sem_destroy(&sem_RR)!=0)
	{
	    perror("Failed to destroy semaphore.");
	    exit(-5);
	}	

	return 0;
}
