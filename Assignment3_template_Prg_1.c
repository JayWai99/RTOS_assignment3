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
#define TIME_QUANTUM 4

typedef struct RR_Params {
  //add your variables here
  int process_id, queue_id;
  int arrive_time, wait_time, burst_time, turnaround_time, remain_time, tick;
  bool running;
} ThreadParams;

sem_t sem_RR;

ThreadParams processes[7];

int i;

float average_wait = 0;

float average_turnaround = 0;

pthread_t thread_A, thread_B;

int active = 0;

int remaining = 0;

void set_input_processes() {
	processes[0].process_id = 1; processes[0].arrive_time = 8; processes[0].burst_time = 10; 
	processes[1].process_id = 2; processes[1].arrive_time = 10; processes[1].burst_time = 3; 
	processes[2].process_id = 3; processes[2].arrive_time = 14; processes[2].burst_time = 7; 
	processes[3].process_id = 4; processes[3].arrive_time = 9; processes[3].burst_time = 5; 
	processes[4].process_id = 5; processes[4].arrive_time = 16; processes[4].burst_time = 4; 
	processes[5].process_id = 6; processes[5].arrive_time = 21; processes[5].burst_time = 6; 
	processes[6].process_id = 7; processes[6].arrive_time = 26; processes[6].burst_time = 2; 

	processes[0].running = false; processes[0].tick = 0; processes[0].queue_id = 0;
	processes[1].running = false; processes[1].tick = 0; processes[1].queue_id = 0;
	processes[2].running = false; processes[2].tick = 0; processes[2].queue_id = 0;
	processes[3].running = false; processes[3].tick = 0; processes[3].queue_id = 0;
	processes[4].running = false; processes[4].tick = 0; processes[4].queue_id = 0;
	processes[5].running = false; processes[5].tick = 0; processes[5].queue_id = 0;
	processes[6].running = false; processes[6].tick = 0; processes[6].queue_id = 0;


     	// remain_time = burst_time at this point because nothing has
		// been processed yet.
	for (i = 0; i < PROCESS_NUMBER; i++) {
		processes[i].remain_time = processes[i].burst_time;
		printf("Process %i has %i seconds left.\n", processes[i].process_id, processes[i].remain_time);
	}
}

void calculate_average(){
	average_wait /= PROCESS_NUMBER;
	average_turnaround /= PROCESS_NUMBER; 
}

void run_process_RR(){
	int end_time, queue = 0, temp, time = 0;
	bool found;
	while (remaining != PROCESS_NUMBER){
		found = false;
		printf("Time: %i\n", time);
		for (int i = 0; i < PROCESS_NUMBER; i++){
			if(processes[i].queue_id > 0){
				continue;
			}			
			else if (time == processes[i].arrive_time && processes[i].running == false && queue == 0){
				processes[i].running = true;
				queue++;
				processes[i].queue_id = queue;
				active = processes[i].queue_id -1;
				printf("Process %i is now running with queue number %i.\n", processes[i].process_id, processes[i].queue_id);
				printf("Process %i with queue number %i has %i seconds remaining.\n", processes[active].process_id, processes[active].queue_id, processes[active].remain_time);
			}
			else if (time == processes[i].arrive_time && processes[i].running == false && queue != 0){
				queue++;
				processes[i].queue_id = queue;
				printf("Process %i has taken a queue number %i and is actively waiting.\n", processes[i].process_id, processes[i].queue_id);
			}
		}
		if (queue == 0){
			time++;
			continue;
		}

		if (processes[active].tick < TIME_QUANTUM && processes[active].remain_time > 0 && processes[active].running == true){
			processes[active].remain_time--;
			printf("Process %i has %i seconds left.\n", processes[active].process_id, processes[active].remain_time);
			processes[active].tick++;
			printf("Process %i tick = %i\n", processes[active].process_id, processes[active].tick);
			time++;
			if (processes[active].tick == TIME_QUANTUM){
				processes[active].running = false;
				processes[active].tick = 0;
				printf("Process %i has finished a cycle.\n", processes[active].process_id);
				if (processes[active].queue_id == PROCESS_NUMBER){
					for (int k = 0; k < PROCESS_NUMBER; k++){
						if (processes[k].queue_id == 1){
							active = k;
							processes[active].running = true;
							printf("Process %i with queue number %i will begin running in the next cycle.\n", processes[active].process_id, processes[active].queue_id);
							printf("Process %i with queue number %i has %i seconds remaining.\n", processes[active].process_id, processes[active].queue_id, processes[active].remain_time);
						}
					}
				}
				else{
					temp = processes[active].queue_id + 1;
					while(found == false){
						for (int j = 0; j < PROCESS_NUMBER; j++){
							if (processes[j].queue_id == temp && processes[j].remain_time != 0){
								active = j;
								processes[active].running = true;
								printf("Process %i with queue number %i will begin running in the next cycle.\n", processes[active].process_id, processes[active].queue_id);
								printf("Process %i with queue number %i has %i seconds remaining.\n", processes[active].process_id, processes[active].queue_id, processes[active].remain_time);
								found = true;
								break;
							}			
						}

						if (temp == PROCESS_NUMBER){
							temp = 1;
						}
						else temp++;						
					}
				}
				printf("active = %i\n", active);

			}
			else if (processes[active].remain_time == 0){
				processes[active].running = false;
				processes[active].tick = 0;
				printf("Process %i has finished running.\n", processes[active].process_id);
				if (remaining != PROCESS_NUMBER){
					if (processes[active].queue_id == PROCESS_NUMBER){
						for (int k = 0; k < PROCESS_NUMBER; k++){
							if (processes[k].queue_id == 1){
								active = k;
								processes[active].running = true;
								printf("Process %i with queue number %i will begin running in the next cycle.\n", processes[active].process_id, processes[active].queue_id);
								printf("Process %i with queue number %i has %i seconds remaining.\n", processes[active].process_id, processes[active].queue_id, processes[active].remain_time);
							}
						}
					}
					else{
						temp = processes[active].queue_id + 1;
						while(found == false){
							for (int j = 0; j < PROCESS_NUMBER; j++){
								if (processes[j].queue_id == temp && processes[j].remain_time != 0){
									active = j;
									processes[active].running = true;
									printf("Process %i with queue number %i will begin running in the next cycle.\n", processes[active].process_id, processes[active].queue_id);
									printf("Process %i with queue number %i has %i seconds remaining.\n", processes[active].process_id, processes[active].queue_id, processes[active].remain_time);
									found = true;
									break;
								}
							}

							if (temp == PROCESS_NUMBER){
								temp = 1;
							}
							else temp++;						
						}
					}
				}
					
				printf("active = %i\n", active);
				remaining++;
			}


	}
	}	
	/*
	for (time = 0; remainder != PROCESS_NUMBER; time++){ 
		printf("active Time: %i\n", time); 
		for (int i = 0; i < PROCESS_NUMBER; i++){
			if (time == processes[i].arrive_time && processes[i].running == false && active == 0){
				processes[i].running == true;
				queue++;
				processes[i].queue_id = queue;
				active = i+1;
				printf("Process %i is now running with queue number %i.\n", processes[i].process_id, processes[i].queue_id);
			}
			else if (time == processes[i].arrive_time && processes[i].running == false && active != 0 & active != processes[i].queue_id){
				queue++;
				processes[i].queue_id = queue;				
				printf("Process %i has taken a queue number %i and is actively waiting.\n", processes[i].process_id, processes[i].queue_id);
			}
			
			else{
				printf("Process %i is not running.\n", i);
			}
		}

		for (int j = 0; j < PROCESS_NUMBER; j++){
			if (j+1 != processes[j].queue_id || processes[j].running != true){
				continue;
			}
			else{
				if (processes[active].tick < TIME_QUANTUM && processes[active].remain_time > 0){
					processes[active].remain_time--;
					processes[active].tick++;
				}
				else if(processes[active].tick >= TIME_QUANTUM && processes[active].remain_time > 0){
					processes[active].tick = 0;
					processes[active].running = false;
				}
				else{
					processes[active].tick = 0;
					processes[active].running = false;
					remainder++;
					end_time = time + 1;
					processes[active].turnaround_time = end_time - processes[active].arrive_time;
					processes[active].wait_time = end_time - processes[active].burst_time - processes[active].arrive_time;
					average_wait += processes[active].wait_time;
					average_turnaround += processes[active].turnaround_time;	
				}
			}	
		}
	}	
	*/
}

//Print results, taken from sample
void print_results() {
	
	printf("Process Schedule Table: \n");
	
	printf("\tProcess ID\tArrival Time\tBurst Time\tWait Time\tTurnaround Time\n");
	
	for (i = 0; i<PROCESS_NUMBER; i++) {
	  	printf("\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n", processes[i].process_id,processes[i].arrive_time, processes[i].burst_time, processes[i].wait_time, processes[i].turnaround_time);
	}
	
	printf("\nAverage wait time: %fs\n", average_wait);
	
	printf("\nAverage turnaround time: %fs\n", average_turnaround);
}

/* this function calculates Round Robin (RR) with a time quantum of 4, writes waiting time and turn-around time to the RR */
void *worker1()
{
   // add your code here
   run_process_RR();
   //calculate_average();
   //fifo_write();
   sem_post(&sem_RR);
}

/* reads the waiting time and turn-around time through the RR and writes to text file */
void *worker2()
{

   // add your code here
   sem_wait(&sem_RR);
   print_results();
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
	
	set_input_processes();
	
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
