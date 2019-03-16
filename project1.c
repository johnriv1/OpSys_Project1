#include <stdio.h>  /* printf */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE, malloc, free */
#include <string.h> /* strlen, strcpy, etc. */
#include <ctype.h> /* isalnum, isdigit */ 
#include <math.h> /* trunc */

#include "process.h"

double exp_dist_eq(double lambda, int upper_bound)
{
	int iterations = 1;
	double x;
  	for ( int i = 0 ; i < iterations ; i++ )
  	{
   	//double lambda = 0.001;  /* average should be 1/lambda ==> 1000 */

    	double r = drand48();   /* uniform dist [0.00,1.00) -- also check out random() */
    	x = -log( r ) / lambda;  /* log() is natural log */

    	/* avoid values that are far down the "long tail" of the distribution */
    	if ( x > upper_bound ) { i--; continue; }
	}
	return x;
}

void paramater_alg(double lambda, int upper_bound, struct process* proc) 
{
	(*proc).arrival_time = floor(exp_dist_eq(lambda, upper_bound));
	(*proc).num_CPU_bursts = (trunc(drand48()*100) + 1);
	(*proc).num_CPU_bursts_remaining = 	(*proc).num_CPU_bursts;
	/*allocate arrays for CPU and IO burst times. Last CPU burst does not have I/O burst time*/
	(*proc).CPU_burst_times = calloc(((*proc).num_CPU_bursts), sizeof(int));
	(*proc).IO_burst_times = calloc(((*proc).num_CPU_bursts-1), sizeof(int));
	for (int i = 0; i < (*proc).num_CPU_bursts; i++)
	{
		((*proc).CPU_burst_times)[i] = ceil(exp_dist_eq(lambda, upper_bound));
		if (i != ((*proc).num_CPU_bursts - 1))
		{
			((*proc).IO_burst_times)[i] = ceil(exp_dist_eq(lambda, upper_bound));
		}
	}
}

void get_next_event(struct process* CPU_BURST_PROCESS, struct process* least_rem_IO_time_Process, struct process* all_processes, int next_arrival_index, int num_processes, int ** next_event_array, int time)
{
	int CPU_burst_completion_time = 0;
	int IO_burst_completion_time = 0;
	int next_arrival_time = 0;
	
	//if there is a process in the CPU
	if (CPU_BURST_PROCESS != NULL)
	{
		CPU_burst_completion_time = (CPU_BURST_PROCESS->CPU_remaining_time) + time;
		#ifdef DEBUG_MODE
		printf ("current process will finish CPU burst at %d ms\n", CPU_burst_completion_time);
		#endif
	}
	//if there is a process doing I/O
	if (least_rem_IO_time_Process != NULL)
	{
		IO_burst_completion_time = (least_rem_IO_time_Process->IO_remaining_time) + time;
		#ifdef DEBUG_MODE
		printf ("current process will finish I/O burst at %d ms\n", IO_burst_completion_time);
		#endif
	}
	//if index is in array (it wouldn't be if we're on last process)
	if (next_arrival_index < num_processes)
	{
		next_arrival_time = all_processes[next_arrival_index].arrival_time;
		#ifdef DEBUG_MODE
		printf ("next process arrives at time %d ms\n", next_arrival_time);
		#endif
	}
	
	//if values don't apply, make them large enough to not be the minimum value
	if (CPU_BURST_PROCESS == NULL)
	{
		CPU_burst_completion_time = IO_burst_completion_time + next_arrival_time + time + 1;
	}
	if (least_rem_IO_time_Process == NULL)
	{
		IO_burst_completion_time = CPU_burst_completion_time + next_arrival_time + time + 1;
	}
	if (next_arrival_index >= num_processes)
	{
		next_arrival_time = CPU_burst_completion_time + IO_burst_completion_time + time + 1;
	}
	
	#ifdef DEBUG_MODE
	printf ("(modified val) current process will finish CPU burst at %d ms\n", CPU_burst_completion_time);
	printf ("(modified val) current process will finish I/O burst at %d ms\n", IO_burst_completion_time);
	printf ("(modified val) next process arrives at time %d ms\n", next_arrival_time);
	#endif
	
	if ((CPU_burst_completion_time <= IO_burst_completion_time) && (CPU_burst_completion_time <= next_arrival_time))
	{
		(*next_event_array)[0] = 1;
	}
	if ((IO_burst_completion_time <= CPU_burst_completion_time) && (IO_burst_completion_time <= next_arrival_time))
	{
		(*next_event_array)[1] = 1;
	}
	if ((next_arrival_time <= CPU_burst_completion_time) && (next_arrival_time <= IO_burst_completion_time))
	{
		(*next_event_array)[2] = 1;
	}
}

void Advance_Ready_Queue( struct process*** READY_QUEUE, int* READY_QUEUE_size )
{
	//READY_QUEUE is an array of pointers to structs. It is pointing to structs in all_processes
	//so don't remove the structs (what they're pointing to), just the pointers themselves. 
	
	#ifdef DEBUG_MODE
	printf("Queue contents before advancement of queue are [Q ");
	for (int i = 0; i < (*READY_QUEUE_size); i++)
	{
		printf("%c ", (*READY_QUEUE)[i]->id); 
	}
	printf("]\n");
	#endif
					
	for (int i = 0; i < ((*READY_QUEUE_size)-1); i++)
	{
		(*READY_QUEUE)[i] = (*READY_QUEUE)[i+1];
	}
	(*READY_QUEUE_size) -= 1;
	(*READY_QUEUE) = realloc(*READY_QUEUE, (*READY_QUEUE_size)*sizeof(struct process *));
	
	#ifdef DEBUG_MODE
	printf("Queue contents after advancement of queue are [Q ");
	for (int i = 0; i < (*READY_QUEUE_size); i++)
	{
		printf("%c ", (*READY_QUEUE)[i]->id); 
	}
	printf("]\n");
	#endif
}

int Add_to_Ready_Queue( struct process*** READY_QUEUE, int * READY_QUEUE_size, char* alg, struct process* incoming_Process)
{
	/*initialize it to arbitrary value, will get changedin one of the if statements*/
	int queue_index=1;
	#ifdef DEBUG_MODE
	printf("Adding process with id %c", incoming_Process->id);
	
	printf("Queue contents before adding to queue are [Q ");
	for (int i = 0; i < (*READY_QUEUE_size); i++)
	{
		printf("%c ", (*READY_QUEUE)[i]->id); 
	}
	printf("]\n");
	#endif
	if (strcmp(alg, "FCFS") == 0)
	{
		//make ready queue one pointer bigger
		(*READY_QUEUE_size) += 1;
		(*READY_QUEUE) = realloc(*READY_QUEUE, (*READY_QUEUE_size)*sizeof(struct process *));
		
		//now last pointer in ready queue should point to the same struct that incoming_Process is pointing to (both point to a struct in all_processes);
		(*READY_QUEUE)[(*READY_QUEUE_size)-1] = incoming_Process;
		queue_index = ((*READY_QUEUE_size)-1);
	}
	#ifdef DEBUG_MODE
	printf("Queue contents after adding to queue are [Q ");
	for (int i = 0; i < (*READY_QUEUE_size); i++)
	{
		printf("%c ", (*READY_QUEUE)[i]->id); 
	}
	printf("]\n");
	#endif
	return queue_index;
}

void update_remaining_times(struct process** CPU_BURST_PROCESS, struct process*** IO_PROCESSES, int time, int IO_PROCESSES_size)
{
	if (*CPU_BURST_PROCESS != NULL)
	{
		//total(original) CPU Burst time - (current time - CPU arrival time)
		(*CPU_BURST_PROCESS)->CPU_remaining_time = (*CPU_BURST_PROCESS)->CPU_burst_times[(*CPU_BURST_PROCESS)->curr_CPU_index] - (time-((*CPU_BURST_PROCESS)->curr_CPU_arrival_time));
	}
	if (IO_PROCESSES_size > 0)
	{
		for (int i = 0; i < IO_PROCESSES_size; i++)
		{
		//total(original) CPU Burst time - (current time - CPU arrival time)
			(*IO_PROCESSES)[i]->IO_remaining_time = ((*IO_PROCESSES)[i]->IO_burst_times[(*IO_PROCESSES)[i]->curr_IO_index]) - (time-((*IO_PROCESSES)[i]->curr_IO_arrival_time));
		}
	}
}

void update_next_IO_finish(struct process** least_rem_IO_time_Process, struct process*** IO_PROCESSES , int IO_PROCESSES_size)
{
	if (IO_PROCESSES_size > 0)
	{
		int min_IO_time_left = (*IO_PROCESSES)[0]->IO_remaining_time;
		(*least_rem_IO_time_Process) = (*IO_PROCESSES)[0];
		for (int i = 0; i < IO_PROCESSES_size; i++)
		{
			if (min_IO_time_left > ((*IO_PROCESSES)[i]->IO_remaining_time))
			{
				//now least_rem_IO_time_Process should point to same struct as (IO_PROCESSES)[i] pointer
				(*least_rem_IO_time_Process) = (*IO_PROCESSES)[i];
			}
		}
	}
}

void print_ready_queue(int READY_QUEUE_size, struct process*** READY_QUEUE)
{
	if (READY_QUEUE_size == 0) 
	{
		printf("[Q <empty>]\n");
	}
	else
	{
		printf("[Q ");
		for (int i = 0; i < READY_QUEUE_size; i++)
		{
			printf("%c ", (*READY_QUEUE)[i]->id);
		}
		printf("]\n");
	}
}

int main( int argc, char** argv ) 
{

	/* SET ARGV[] VARIABLES AND ERROR CHECKING *///////////////////////////////////////////////////
	
   /* Two additional command line arguments: size of the cache and the text file*/
   if (!(( argc == 9 ) || (argc == 8)))
   {
      fprintf( stderr, "ERROR: Invalid arguments\n" );
      return EXIT_FAILURE;
   }
	/* argv[4] is number of processes */
	/* Verify that arguments are integers */
	for (int i = 1; i < argc; i++)
	{
		for (int j=0; j < strlen(argv[i]); j++) 
		{
			if (i != 8)
			{
				if ((!isdigit(argv[i][j])) && (argv[i][j] != '.'))
				{
					fprintf( stderr, "ERROR: Invalid arguments\n" );
					if (i == 1)
					{
						fprintf( stderr, "USAGE: %s should be seed\n", argv[i] );
					}
					else if (i == 2)
					{
						fprintf( stderr, "USAGE: %s should be paramater lambda\n", argv[i] );
					}
					else if (i == 3)
					{
						fprintf( stderr, "USAGE: %s should be upper bound\n", argv[i] );
					}
					else if (i == 4)
					{
						fprintf( stderr, "USAGE: %s should be number of processes \n", argv[i] );
					}
					else if (i == 5)
					{
						fprintf( stderr, "USAGE: %s should be context switch time \n", argv[i] );
					}
				  	else if (i == 6)
					{
						fprintf( stderr, "USAGE: %s should be alpha \n", argv[i] );
					}
					else if (i == 7)
					{
						fprintf( stderr, "USAGE: %s should be time slice \n", argv[i] );
					}
					return EXIT_FAILURE;
				}
			}
		}
	}

	char* RRadd;
	if (argc != 9)
	{
		RRadd = "END";
	}
	else if (strcmp(argv[8],"BEGINNING") == 0)
	{
		RRadd = "BEGINNING";
	}
	else if (strcmp(argv[8],"END") == 0)
	{
		RRadd = "END";
	}
	else 
	{
		fprintf( stderr, "USAGE: %s should be \"BEGINNING\" or \"END\"\n", argv[8] );
		return EXIT_FAILURE;
	}
	
   int num_processes = atoi(argv[4]);
   /* Verify that num of processes argument is a integer in range */
	if ((num_processes > 26) || (num_processes < 1))
	{
		fprintf( stderr, "ERROR: Invalid arguments\n" );
		fprintf( stderr, "USAGE: argv[4] should be at most 26 and greater than 0\n");
		return EXIT_FAILURE;
	}
	
	int seed = atoi(argv[1]);
	double lambda = atof(argv[2]);
	int upper_bound = atoi(argv[3]);
	int context_switch_time = atoi(argv[5]);
	int alpha = atoi(argv[6]);
	int time_slice = atoi(argv[7]);
	
	//for each process, get 4 random uniform distribution numbers from .1 to 1.0
	//To get process arrival time, get first random number, plug it into exp-random.c
	//To get number of CPU bursts, the second uniform distibution number is multiplied y 100, truncated, and incremented by 1
	//For each of these cpu bursts, identify the actual CPU burst time and the I/O burst time asthe next two random numbers in the sequence; for the last CPU burst, do not generate anI/O burst time
	
	srand48( seed );
	struct process* all_processes = calloc(num_processes, sizeof(struct process));
	for (int i = 0, let = 'A'; i < num_processes; i++, let++)
	{
		all_processes[i].id = let;
		paramater_alg(lambda, upper_bound, &all_processes[i]);
	}
	#ifdef DEBUG_MODE
	for (int i = 0; i < num_processes; i++)
	{
		printf("PROCESS %c [NEW] (arrival time %d ms) %d CPU bursts\n", all_processes[i].id, all_processes[i].arrival_time, all_processes[i].num_CPU_bursts);
		for (int j = 0; j < all_processes[i].num_CPU_bursts; j++)
		{
			if (j != ((all_processes[i].num_CPU_bursts) - 1))
			{
				printf("--> CPU burst %d ms --> I/O burst %d ms\n", (all_processes[i].CPU_burst_times)[j], (all_processes[i].IO_burst_times)[j]);
			}
			else
			{
				printf("--> CPU burst %d ms\n", (all_processes[i].CPU_burst_times)[j]);
			}
		}
	}
	#endif
	
	//array of pointers to process in ready queue order
	struct process** READY_QUEUE = NULL;
	int READY_QUEUE_size = 0;
	//array of pointers to process that are doing IO
	struct process** IO_PROCESSES = NULL;
	int IO_PROCESSES_size = 0;
	//points to process that is currently in CPU
	struct process* CPU_BURST_PROCESS = NULL;
	//points to process with least remaining I/O time
	struct process* least_rem_IO_time_Process = NULL;
	//sort all_processes by order of arrival
	qsort (all_processes, num_processes, sizeof(struct process), compare_arrival_time);
	#ifdef DEBUG_MODE
	for (int i = 0; i < num_processes; i++)
	{
		printf("PROCESS %c [NEW] (arrival time %d ms) %d CPU bursts\n", all_processes[i].id, all_processes[i].arrival_time, all_processes[i].num_CPU_bursts);
	}
	#endif
	//index of next arriving process. This will eventually traverse all_processes array
	int next_arrival_index=0;
	//will set this to 1 when last burst of last process is complete
	int finished = 0;
	//time passed since start of the algorithm
	int time = 0;
	int queue_index;
	int test_int = 0;
	
	/* FIRST COME FIRST SERVE *////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	//while (!finished)
	while (test_int != 5)
	{
		test_int++;
		//first process to come in
		if (READY_QUEUE_size == 0 && IO_PROCESSES_size == 0 && CPU_BURST_PROCESS == NULL)
		{
			//first process to arrive occupies the cpu
			CPU_BURST_PROCESS = &all_processes[0];
			time += (CPU_BURST_PROCESS->arrival_time);
			printf("time %dms: Process %c arrived; added to ready queue ", time, all_processes[next_arrival_index].id);
			print_ready_queue(READY_QUEUE_size, &READY_QUEUE);
			//update time to first arrival
			time += context_switch_time/2;
			//doing the first CPU burst in CPU burst array and set remaining time to be whole CPU burst time
			
			CPU_BURST_PROCESS->curr_CPU_index = 0;
			CPU_BURST_PROCESS->CPU_remaining_time = CPU_BURST_PROCESS->CPU_burst_times[0];
			printf("time %dms: Process %c started using the CPU for %dms burst ", time, CPU_BURST_PROCESS->id, CPU_BURST_PROCESS->CPU_burst_times[CPU_BURST_PROCESS->curr_CPU_index]);
			print_ready_queue(READY_QUEUE_size, &READY_QUEUE);
			CPU_BURST_PROCESS->curr_CPU_arrival_time = time;
			next_arrival_index+=1;
			/*
			printf("First in CPU is process %c with burst time of %d\n", CPU_BURST_PROCESS->id, CPU_BURST_PROCESS->CPU_remaining_time);
			finished = 1;
			*/
		}
		else
		{
			/*get next event(s)
			 function returns array that will only have multiple entries if multiple events occur at once*/
			int* next_event_array = calloc(3, sizeof(int));
			get_next_event(CPU_BURST_PROCESS, least_rem_IO_time_Process, all_processes, next_arrival_index, num_processes, &next_event_array, time);
			#ifdef DEBUG_MODE
			printf("next_event_array is [%d, %d, %d]\n", next_event_array[0],next_event_array[1], next_event_array[2]);
			#endif
			
			/*next event is CPU burst finishes*/
			if (next_event_array[0] == 1)
			{
				/*increase time by the remaining burst time and 
				bring CPU process remaining time down to 0*/
				time += CPU_BURST_PROCESS->CPU_remaining_time;
				CPU_BURST_PROCESS->CPU_remaining_time -= CPU_BURST_PROCESS->CPU_remaining_time;
				CPU_BURST_PROCESS->num_CPU_bursts_remaining -= 1;
				printf("time %dms: Process %c completed a CPU burst; %d bursts to go ", time, CPU_BURST_PROCESS->id, CPU_BURST_PROCESS->num_CPU_bursts_remaining);
				print_ready_queue(READY_QUEUE_size, &READY_QUEUE);
				/*increment CPU burst index and set remaining time to new corresponding CPU burst*/
				/*only do this if there is a next CPU burst*/
				/*also, if not last burst, process does IO burst, 
					which is denoted by it going into I/O processes array*/
				if (CPU_BURST_PROCESS->num_CPU_bursts_remaining > 0 )
				{
					CPU_BURST_PROCESS->curr_IO_index = CPU_BURST_PROCESS->curr_CPU_index;
					CPU_BURST_PROCESS->IO_remaining_time = CPU_BURST_PROCESS->IO_burst_times[CPU_BURST_PROCESS->curr_IO_index];
					
					printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms ", 
						time, CPU_BURST_PROCESS->id, (time + (context_switch_time/2) + CPU_BURST_PROCESS->IO_remaining_time));
					
					CPU_BURST_PROCESS->curr_CPU_index += 1;
					CPU_BURST_PROCESS->CPU_remaining_time = CPU_BURST_PROCESS->CPU_burst_times[CPU_BURST_PROCESS->curr_CPU_index];
					time += (context_switch_time)/2;
					
					/*switch process out of CPU and into I/O*/
					IO_PROCESSES_size += 1;
					if (IO_PROCESSES == NULL)
					{
						IO_PROCESSES = calloc(IO_PROCESSES_size, sizeof(struct process *));
					}
					else
					{
						IO_PROCESSES = realloc(IO_PROCESSES, IO_PROCESSES_size * sizeof(struct process *));
					}
					IO_PROCESSES[IO_PROCESSES_size-1] = CPU_BURST_PROCESS;
					IO_PROCESSES[IO_PROCESSES_size-1]->curr_IO_arrival_time = time;
					
					CPU_BURST_PROCESS = NULL;
				}
				/*if ready Queue has a process ready, put it into the CPU*/
				if (	READY_QUEUE_size > 0 )
				{
					CPU_BURST_PROCESS = READY_QUEUE[0];
					time += (context_switch_time)/2;
					CPU_BURST_PROCESS->curr_CPU_arrival_time = time;
					Advance_Ready_Queue(&READY_QUEUE, &READY_QUEUE_size);
					printf("time %dms: Process %c started using the CPU for %dms burst ", time, CPU_BURST_PROCESS->id, CPU_BURST_PROCESS->CPU_burst_times[CPU_BURST_PROCESS->curr_CPU_index]);
					print_ready_queue(READY_QUEUE_size, &READY_QUEUE);
				}
			}
			finished = 1;
			/*next event is PROCESS I/O FINISHES*/
			if (next_event_array[1] == 1)
			{
				//put it on to the ready queue. Logic for putting it into the queue will change in each alg.
				printf("time %dms: Process %c completed I/O; added to ready queue ", time, least_rem_IO_time_Process->id);
				time += (context_switch_time)/2;
				queue_index = Add_to_Ready_Queue(&READY_QUEUE, &READY_QUEUE_size, "FCFS", least_rem_IO_time_Process);
				//find and take process out of IO array
				for (int i = 0; i < IO_PROCESSES_size; i++)
				{
					if (least_rem_IO_time_Process->id == IO_PROCESSES[i]->id)
					{
						for (int j = i; j < (IO_PROCESSES_size-1); j++)
						{
							(IO_PROCESSES)[i] = (IO_PROCESSES)[i+1];
						}
						(IO_PROCESSES_size) -= 1;
						(IO_PROCESSES) = realloc(IO_PROCESSES, IO_PROCESSES_size*sizeof(struct process *));
						break;
					}
				}
				print_ready_queue(READY_QUEUE_size, &READY_QUEUE);
				//if new process in front of ready queue and CPU is free, put process into CPU and take it out of the ready queue 
				if ((queue_index == 0) && (CPU_BURST_PROCESS == NULL))
				{
					CPU_BURST_PROCESS = READY_QUEUE[0];
					time += (context_switch_time)/2;
					CPU_BURST_PROCESS->curr_CPU_arrival_time = time;
					Advance_Ready_Queue(&READY_QUEUE, &READY_QUEUE_size);
					printf("time %dms: Process %c started using the CPU for %dms burst ", time, CPU_BURST_PROCESS->id, CPU_BURST_PROCESS->CPU_burst_times[CPU_BURST_PROCESS->curr_CPU_index]);
					print_ready_queue(READY_QUEUE_size, &READY_QUEUE);
				}
			}
			/*next event is process arrival*/
			if (next_event_array[2] == 1)
			{
				//put it on to the ready queue. Logic for putting it into the queue will change in each alg.
				queue_index = Add_to_Ready_Queue(&READY_QUEUE, &READY_QUEUE_size, "FCFS", &all_processes[next_arrival_index]);
				all_processes[next_arrival_index].curr_CPU_index = 0;
				all_processes[next_arrival_index].CPU_remaining_time = all_processes[next_arrival_index].CPU_burst_times[0];
				printf("time %dms: Process %c arrived; added to ready queue ", time, all_processes[next_arrival_index].id);
				print_ready_queue(READY_QUEUE_size, &READY_QUEUE);
				next_arrival_index += 1;
				//if new process in front of ready queue and CPU is free, put process into CPU and take it out of the ready queue 
				if ((queue_index == 0) && (CPU_BURST_PROCESS == NULL))
				{
					CPU_BURST_PROCESS = READY_QUEUE[0];
					time += (context_switch_time)/2;
					CPU_BURST_PROCESS->curr_CPU_arrival_time = time;
					Advance_Ready_Queue(&READY_QUEUE, &READY_QUEUE_size);
					printf("time %dms: Process %c started using the CPU for %dms burst ", time, CPU_BURST_PROCESS->id, CPU_BURST_PROCESS->CPU_burst_times[CPU_BURST_PROCESS->curr_CPU_index]);
					print_ready_queue(READY_QUEUE_size, &READY_QUEUE);
				}
			}
			update_remaining_times(&CPU_BURST_PROCESS, &IO_PROCESSES, time, IO_PROCESSES_size);
			update_next_IO_finish(&least_rem_IO_time_Process, &IO_PROCESSES, IO_PROCESSES_size);
		}
		/*check to see if we're done*/
		if (READY_QUEUE_size == 0 && IO_PROCESSES_size == 0 && CPU_BURST_PROCESS == NULL && next_arrival_index >= num_processes)
		{
			finished = 1;
		}
	}
}




