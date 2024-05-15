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
  int arrive_time, wait_time, burst_time, turnaround_time, remain_time, tick, start_time;
  bool running;
} ThreadParams;

sem_t sem_RR, sem_FIFO;

char output_file[50];

char* fifo_file = "./fifoAssignment3";

ThreadParams processes[7];

int i;

float total_wait = 0;

float average_wait = 0;

float total_turnaround = 0;

float average_turnaround = 0;

pthread_t thread_A, thread_B;

int active = 0, queue = 0;

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

	processes[0].start_time = 0; processes[0].wait_time = 0;
	processes[1].start_time = 0; processes[1].wait_time = 0;
	processes[2].start_time = 0; processes[2].wait_time = 0;
	processes[3].start_time = 0; processes[3].wait_time = 0;
	processes[4].start_time = 0; processes[4].wait_time = 0;
	processes[5].start_time = 0; processes[5].wait_time = 0;
	processes[6].start_time = 0; processes[6].wait_time = 0;

     	// remain_time = burst_time at this point because nothing has
		// been processed yet.
	for (i = 0; i < PROCESS_NUMBER; i++) {
		processes[i].remain_time = processes[i].burst_time;
		printf("Process %i has %i milliseconds left.\n", processes[i].process_id, processes[i].remain_time);
	}
}

void assign_queue(int counter){
	for (int i = 0; i < PROCESS_NUMBER; i++){
		if(processes[i].queue_id > 0){
			continue;
		}			
		else if (counter == processes[i].arrive_time && processes[i].running == false && queue == 0){
			processes[i].running = true;
			queue++;
			processes[i].queue_id = queue;
			active = processes[i].queue_id -1;
			printf("Process %i is now running with queue number %i.\n", processes[i].process_id, processes[i].queue_id);
			printf("Process %i with queue number %i has %i milliseconds remaining.\n", processes[active].process_id, processes[active].queue_id, processes[active].remain_time);
		}
		else if (counter == processes[i].arrive_time && processes[i].running == false && queue != 0){
			queue++;
			processes[i].queue_id = queue;
			printf("Process %i has taken a queue number %i and is actively waiting.\n", processes[i].process_id, processes[i].queue_id);
		}
	}
}

void increment_wait_time(){
	for (int i = 0; i < PROCESS_NUMBER; i++){
		if(processes[i].running == false && processes[i].remain_time > 0){
			processes[i].wait_time++;
		}
	}	
}

void increment_turnaround_time(){
	for (int i = 0; i < PROCESS_NUMBER; i++){
		if (processes[i].remain_time > 0){
			processes[i].turnaround_time++;
		}
	}
}

int scheduler(int counter, bool found){
	int temp;
	if (processes[active].tick < TIME_QUANTUM && processes[active].remain_time > 0 && processes[active].running == true){
		processes[active].remain_time--;
		printf("Process %i has %i milliseconds left.\n", processes[active].process_id, processes[active].remain_time);
		processes[active].tick++;
		printf("Process %i tick = %i\n", processes[active].process_id, processes[active].tick);
		counter++;
		if (processes[active].tick == TIME_QUANTUM){
			processes[active].running = false;
			processes[active].tick = 0;
			if (processes[active].remain_time == 0){
				printf("Process %i has finished running.\n", processes[active].process_id);
				remaining++;
			}
				
			else printf("Process %i has finished a cycle.\n", processes[active].process_id);
				
			if (processes[active].queue_id == PROCESS_NUMBER){
				for (int k = 0; k < PROCESS_NUMBER; k++){
					if (processes[k].queue_id == 1){
						active = k;
						processes[active].running = true;
						printf("Process %i with queue number %i will begin running in the next cycle.\n", processes[active].process_id, processes[active].queue_id);
						printf("Process %i with queue number %i has %i milliseconds remaining.\n", processes[active].process_id, processes[active].queue_id, processes[active].remain_time);
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
							printf("Process %i with queue number %i has %i milliseconds remaining.\n", processes[active].process_id, processes[active].queue_id, processes[active].remain_time);
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
			remaining++;
			printf("remainder = %i\n", remaining);
			if (remaining != PROCESS_NUMBER){
				if (processes[active].queue_id == PROCESS_NUMBER){
					for (int k = 0; k < PROCESS_NUMBER; k++){
						if (processes[k].queue_id == 1){
							active = k;
							processes[active].running = true;
							printf("Process %i with queue number %i will begin running in the next cycle.\n", processes[active].process_id, processes[active].queue_id);
							printf("Process %i with queue number %i has %i milliseconds remaining.\n", processes[active].process_id, processes[active].queue_id, processes[active].remain_time);
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
								printf("Process %i with queue number %i has %i milliseconds remaining.\n", processes[active].process_id, processes[active].queue_id, processes[active].remain_time);
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
			else{
				for (int i = 0; i < PROCESS_NUMBER; i++){
					total_turnaround += processes[i].turnaround_time;
					total_wait += processes[i].wait_time;
				}
					
			}
			printf("active = %i\n", active);
		}
	}
	return counter;
}

void run_process_RR(){
	int time = 0;
	bool found;
	while (remaining != PROCESS_NUMBER){
		found = false;
		printf("Time: %i\n", time);

		assign_queue(time);

		if (queue == 0){
			increment_wait_time();
			increment_turnaround_time();
			time++;
			continue;
		}

		increment_wait_time();
		increment_turnaround_time();
		time = scheduler(time, found);
	}
}

void calculate_average(){
	average_wait = total_wait/PROCESS_NUMBER;
	average_turnaround = total_turnaround/PROCESS_NUMBER; 
}


void fifo_write(){
	char* wait_title = "Average Wait Time = ";
	char* turnaround_title = "Average Turnaround Time = ";
	char wait_value[20];
	char turnaround_value[20];
	char* unit = " milliseconds\n";
	char output[100];

	gcvt(average_wait, 8, wait_value);
	gcvt(average_turnaround, 8, turnaround_value);

	strcat(output, wait_title);
	strcat(output, wait_value);
	strcat(output, unit);
	strcat(output, turnaround_title);
	strcat(output, turnaround_value);
	strcat(output, unit);
	// printf("%s", output);
	// printf("%i\n", strlen(output));
	int file_descriptor;
	int result = 0 ;
		
	while(true){
		file_descriptor = open(fifo_file, O_WRONLY);
		if (file_descriptor == -1){
			perror("Error opening file.\n");
			exit(-7);
		}

		printf("Writing results to FIFO...\n");
		// do
		// {
		// 	result = write(file_descriptor, output, strlen(output));
		
		// } while (result < strlen(output));

		write(file_descriptor, output, strlen(output)+1);

		// sem_post(&sem_FIFO);
		close(file_descriptor);

		printf("Results have been written into FIFO.\n");
		break;
	}
}

const char* fifo_read(){
	int file_descriptor, result = 0;

	static char fifo_output[100];

	while(true){

		file_descriptor = open(fifo_file, O_RDONLY);
		if (file_descriptor == -1){
			perror("Error opening file.\n");
			exit(-7);
		}

		printf("Thread B is waiting for data from FIFO...\n");
		// sem_wait(&sem_FIFO);
		sleep(5);		
		// do
		// {
		// 	result = read(file_descriptor, fifo_output, strlen(fifo_output));
		// } while (result < strlen(fifo_output));
		
		// if (result == -1){
		// 	fprintf( stdout, "%s\n", strerror( errno ));
		// 	// perror("Error occurred when attempting to read from FIFO.\n");
		// 	exit(-8);
		// }

		read(file_descriptor, fifo_output, strlen(fifo_output));

		close(file_descriptor);

		
		
		printf("Results have been read from FIFO.\n");
	break;
		
	}

	printf("%s\n", fifo_output);
	return fifo_output;
}

void write_file(const char* fifo_string){
  FILE* fptr = fopen(output_file, "w");
  char* file_content;
//   char text[100];

  if (fptr == NULL){
    printf("Failed to open file.");
    exit(-10);
  }

//   strcpy(text, fifo_string);

  fprintf(fptr, "%s", fifo_string);
  fclose(fptr);

  printf("Finished.\n");
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
   calculate_average();
//    const char* output = generate_output_string();
   sem_post(&sem_RR); 
   fifo_write();
}

/* reads the waiting time and turn-around time through the RR and writes to text file */
void *worker2()
{

   // add your code here
	sem_wait(&sem_RR);
	const char* output = fifo_read();
	//print_results();
	write_file(output);
}

/* this main function creates named pipe and threads */
int main(int argc, char* argv[]){
	if(argc<3){
	    perror("usage: ./program_name 4 output.txt  \n<note: Run the above line of code, which is part of the requirements of Assessment Task 3.>\n");
   		exit(-1);
  	}

	if(atoi(argv[1]) != 4){
		perror("Please make sure the second argument when executing the program is 4 because it is one of the requirements of the Assignment 3.");
		exit(-1);
	}

	strcpy(output_file, argv[2]);
	/* creating a named pipe(RR) with read/write permission */
	// add your code 

	if (access(fifo_file, F_OK) != 0){
		if (mkfifo(fifo_file, 0666) != 0){
		perror("Failed to create named pipe.");
		exit(-2);
		}
	}
	
	/* initialize the parameters */
	 // add your code 
	
	set_input_processes();
	
	if (sem_init(&sem_RR, 0, 0) != 0){
		perror("Failed to initialise semaphore.");
		exit(-3);
	}

	if (sem_init(&sem_FIFO, 0, 0) != 0){
		perror("Failed to initialise semaphore.");
		exit(-3);
	}

	/* create threads */
	 // add your code
	if (pthread_create(&thread_A, NULL, worker1, NULL)){
		perror("Failed to create thread.");
		exit(-4);
	}
	
	if (pthread_create(&thread_B, NULL, worker2, NULL)){
		perror("Failed to create thread.");
		exit(-4);
	}

	/* wait for the thread to exit */
	//add your code
	if(pthread_join(thread_A, NULL)!=0){
	    perror("Failed to join thread.");
		exit(-5);
	}
	if(pthread_join(thread_B, NULL)!=0){
	    perror("Failed to join thread.");
		exit(-5);
	}

	if (unlink(fifo_file) != 0){
		perror("Failed to unlink FIFO.");
	}
	

	if(sem_destroy(&sem_RR)!=0){
	    perror("Failed to destroy semaphore.");
	    exit(-6);
	}	

	return 0;
}
