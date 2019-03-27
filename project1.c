/*
Team Members:
John Rivera (riverj5)
Qixiang Chen (chenq7)
Andrew Gaudet (gaudea)
*/

#include <stdio.h>  /* printf */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE, malloc, free */
#include <string.h> /* strlen, strcpy, etc. */
#include <ctype.h> /* isalnum, isdigit */ 
#include <math.h> /* trunc */

#include "process_3.h"
/*
Sample output:
a.out 2 0.01 200 1 4 0.5 120 > output02.txt
a.out 2 0.01 200 2 4 0.5 120 > output03.txt
a.out 2 0.01 200 12 4 0.5 120 > output04.txt
a.out 73 0.001 3000 12 4 0.5 950 > output05.txt
*/

double exp_dist_eq(double lambda, int upper_bound)
{
	int iterations = 1;
	double x;
  	for ( int i = 0 ; i < iterations ; i++ )
  	{

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
	(*proc).tau = (int)(1/lambda);
	(*proc).tau_remaining = (*proc).tau;
	(*proc).wait_time = 0;
	(*proc).turnaround_time = 0;
	(*proc).is_preempt = 0;
	(*proc).curr_CPU_index = 0;
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
	(*proc).terminated = 0;
}

void get_next_event(struct process* CPU_BURST_PROCESS, struct process* least_rem_IO_time_Process, struct process* all_processes,
 struct process* outof_CPU_switch, struct process* into_CPU_switch, int next_arrival_index, int num_processes, int ** next_event_array, int time, int time_slice_rem, char* alg)
{
	int CPU_burst_completion_time = 0;
	int IO_burst_completion_time = 0;
	int next_arrival_time = 0;
	int outof_CPU_cs_completion_time = 0;
	int into_CPU_cs_completion_time = 0;
	int time_slice_completion_time = 0;
	
	char outof_CPU_switch_id;
	if (outof_CPU_switch == NULL)
	{
		outof_CPU_switch_id = '.';
	}
	else
	{
		outof_CPU_switch_id = outof_CPU_switch->id;
	}
	char into_CPU_switch_id;
	if (into_CPU_switch == NULL)
	{
		into_CPU_switch_id = '.';
	}
	else
	{
		into_CPU_switch_id = into_CPU_switch->id;
	}
	
	//if there is a process in the CPU
	if (CPU_BURST_PROCESS != NULL && CPU_BURST_PROCESS->id != outof_CPU_switch_id && CPU_BURST_PROCESS->id != into_CPU_switch_id)
	{
		CPU_burst_completion_time = (CPU_BURST_PROCESS->CPU_remaining_time) + time;
		#ifdef DEBUG_MODE
		printf ("   current process will finish CPU burst at %d ms\n", CPU_burst_completion_time);
		#endif
	}
	//if there is a process doing I/O
	if (least_rem_IO_time_Process != NULL)
	{
		IO_burst_completion_time = (least_rem_IO_time_Process->IO_remaining_time) + time;
		#ifdef DEBUG_MODE
		printf ("   current process (%c) will finish I/O burst at %d ms\n", least_rem_IO_time_Process->id, IO_burst_completion_time);
		#endif
	}
	//if index is in array (it wouldn't be if we're on last process)
	if (next_arrival_index < num_processes)
	{
		next_arrival_time = all_processes[next_arrival_index].arrival_time;
		#ifdef DEBUG_MODE
		printf ("   next process arrives at time %d ms\n", next_arrival_time);
		#endif
	}
	if (outof_CPU_switch != NULL)
	{
		outof_CPU_cs_completion_time = outof_CPU_switch->switch_remaining_time + time;
	}
	if (into_CPU_switch != NULL)
	{
		into_CPU_cs_completion_time = into_CPU_switch->switch_remaining_time + time;
	}
	if (outof_CPU_switch == NULL && into_CPU_switch == NULL && CPU_BURST_PROCESS != NULL && (strcmp(alg, "RR") == 0))
	{
		time_slice_completion_time = time_slice_rem + time;
	}
	
	int max_val = 	CPU_burst_completion_time + IO_burst_completion_time + next_arrival_time + outof_CPU_cs_completion_time + into_CPU_cs_completion_time + time_slice_rem + 1;
	//if values don't apply, make them large enough to not be the minimum value
	if (!(CPU_BURST_PROCESS != NULL && CPU_BURST_PROCESS->id != outof_CPU_switch_id && CPU_BURST_PROCESS->id != into_CPU_switch_id))
	{
		CPU_burst_completion_time = max_val;
	}
	if (least_rem_IO_time_Process == NULL)
	{
		IO_burst_completion_time = max_val;
	}
	if (next_arrival_index >= num_processes)
	{
		next_arrival_time = max_val;
	}
	if (outof_CPU_switch == NULL)
	{
		outof_CPU_cs_completion_time = max_val;
	}
	if (into_CPU_switch == NULL)
	{
		into_CPU_cs_completion_time = max_val;
	}
	/*if not RR, set this value to max everytime*/
	if (!(outof_CPU_switch == NULL && into_CPU_switch == NULL && CPU_BURST_PROCESS != NULL && (strcmp(alg, "RR") == 0)))
	{
		time_slice_completion_time = max_val;
	}
	
	#ifdef DEBUG_MODE
	printf ("(modified val) current process will finish CPU burst at %d ms\n", CPU_burst_completion_time);
	printf ("(modified val) current process will finish I/O burst at %d ms\n", IO_burst_completion_time);
	printf ("(modified val) next process arrives at time %d ms\n", next_arrival_time);
	printf ("(modified val) current process will finish in-to-CPU burst at %d ms\n", into_CPU_cs_completion_time);
	printf ("(modified val) current process will finish out-of-CPU burst at %d ms\n", outof_CPU_cs_completion_time);
	#endif
	
	if ((CPU_burst_completion_time <= IO_burst_completion_time) && (CPU_burst_completion_time <= next_arrival_time) && (CPU_burst_completion_time <= outof_CPU_cs_completion_time) && (CPU_burst_completion_time <= into_CPU_cs_completion_time) && (CPU_burst_completion_time <= time_slice_completion_time))
	{
		(*next_event_array)[0] = 1;
	}
	if ((IO_burst_completion_time <= CPU_burst_completion_time) && (IO_burst_completion_time <= next_arrival_time) && (IO_burst_completion_time <= outof_CPU_cs_completion_time) && (IO_burst_completion_time <= into_CPU_cs_completion_time) && (IO_burst_completion_time <= time_slice_completion_time))
	{
		(*next_event_array)[1] = 1;
	}
	if ((next_arrival_time <= CPU_burst_completion_time) && (next_arrival_time <= IO_burst_completion_time) && (next_arrival_time <= outof_CPU_cs_completion_time) && (next_arrival_time <= into_CPU_cs_completion_time) && (next_arrival_time <= time_slice_completion_time))
	{
		(*next_event_array)[2] = 1;
	}
	if ((into_CPU_cs_completion_time <= CPU_burst_completion_time) && (into_CPU_cs_completion_time <= IO_burst_completion_time) && (into_CPU_cs_completion_time <= next_arrival_time) && (into_CPU_cs_completion_time <= outof_CPU_cs_completion_time) && (into_CPU_cs_completion_time <= time_slice_completion_time))
	{
		(*next_event_array)[3] = 1;
	}
	if ((outof_CPU_cs_completion_time <= CPU_burst_completion_time) && (outof_CPU_cs_completion_time <= IO_burst_completion_time) && (outof_CPU_cs_completion_time <= next_arrival_time) && (outof_CPU_cs_completion_time <= into_CPU_cs_completion_time) && (outof_CPU_cs_completion_time <= time_slice_completion_time))
	{
		(*next_event_array)[4] = 1;
	}
	if (strcmp(alg, "RR") == 0)   // if equals RR algorithm  
	{
		if ((time_slice_completion_time <= CPU_burst_completion_time) && (time_slice_completion_time <= IO_burst_completion_time) && (time_slice_completion_time <= next_arrival_time) && (time_slice_completion_time <= outof_CPU_cs_completion_time) && (time_slice_completion_time <= into_CPU_cs_completion_time))
		{
			(*next_event_array)[5] = 1;
		}
		/*if CPU burst finishes at same time as timeslice, timeslice event is ignored*/
		if ((*next_event_array)[0] == 1)
		{
			(*next_event_array)[5] = 0;
		}
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


int Add_to_Ready_Queue( struct process*** READY_QUEUE, int * READY_QUEUE_size, char* alg, struct process* incoming_Process, char* algorithm)
{
	/*initialize it to arbitrary value, will get changed in one of the if statements*/
	#ifdef DEBUG_MODE
	printf("Adding process with id %c\n", incoming_Process->id);
	
	printf("Queue contents before adding to queue are [Q ");
	for (int i = 0; i < (*READY_QUEUE_size); i++)
	{
		printf("%c ", (*READY_QUEUE)[i]->id); 
	}
	printf("]\n");
	#endif
	
	int queue_index = (*READY_QUEUE_size);
	if (strcmp(algorithm, "SJF") == 0 || strcmp(algorithm, "SRT") == 0)
	{
		int incoming_tau = incoming_Process -> tau_remaining;
		for (int i = 0; i < (*READY_QUEUE_size); i++)
		{
			if ((*READY_QUEUE)[i]->tau_remaining > incoming_tau)
			{
				queue_index = i;
				break;
			}
			else if ((*READY_QUEUE)[i]->tau_remaining == incoming_tau)
			{
				if ((*READY_QUEUE)[i]->id > incoming_Process->id)
				{
					queue_index = i;
					break;
				}
				/*
				else
				{
					queue_index = i+1;
					break;
				}
				*/
			}
		}
		//printf("Insert Process %c into index %d\n", incoming_Process->id, queue_index); 
		(*READY_QUEUE) = realloc(*READY_QUEUE, (*READY_QUEUE_size+1)*sizeof(struct process *));

		/*shift everything over by 1, then insert new process into queue_indexth position*/
		for (int i = (*READY_QUEUE_size)-1; i >= queue_index; i--)
		{
			//printf("PROCESS IN INDEX %d PROCESS IS NOW IN INDEX %d\n", i, i+1);
			(*READY_QUEUE)[i+1] = (*READY_QUEUE)[i];
		}
		//printf("SHIFTED EVERYTHING IN READY QUEUE\n");
		(*READY_QUEUE_size) += 1;
		(*READY_QUEUE)[queue_index] = incoming_Process;
		//printf("ADDED PROCESS %c TO FRONT OF QUEUE\n", (*READY_QUEUE)[0]->id);
	}
	else
	{
		if (strcmp(alg, "END") == 0)
		{
			//make ready queue one pointer bigger
			(*READY_QUEUE_size) += 1;
			(*READY_QUEUE) = realloc(*READY_QUEUE, (*READY_QUEUE_size)*sizeof(struct process *));
			
			//now last pointer in ready queue should point to the same struct that incoming_Process is pointing to (both point to a struct in all_processes);
			(*READY_QUEUE)[(*READY_QUEUE_size)-1] = incoming_Process;
			queue_index = ((*READY_QUEUE_size)-1);
		}
		else if (strcmp(alg, "BEGINNING") == 0)
		{
			(*READY_QUEUE) = realloc(*READY_QUEUE, (*READY_QUEUE_size+1)*sizeof(struct process *));
			
			/*shift everything over by 1, then insert new process into 0th position*/
			for (int i = (*READY_QUEUE_size)-1; i >= 0; i--)
			{
				//printf("PROCESS IN INDEX %d PROCESS IS NOW IN INDEX %d\n", i, i+1);
				(*READY_QUEUE)[i+1] = (*READY_QUEUE)[i];
			}
			//printf("SHIFTED EVERYTHING IN READY QUEUE\n");
			(*READY_QUEUE_size) += 1;
			(*READY_QUEUE)[0] = incoming_Process;
			//printf("ADDED PROCESS %c TO FRONT OF QUEUE\n", (*READY_QUEUE)[0]->id);
			queue_index = 0;
			/*
			printf("Queue contents after adding to queue are [Q ");
			for (int i = 0; i < (*READY_QUEUE_size); i++)
			{
				printf("%c ", (*READY_QUEUE)[i]->id); 
			}
			printf("]\n");
			*/
		}
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

void update_remaining_times(struct process** CPU_BURST_PROCESS, struct process*** IO_PROCESSES, int time, int IO_PROCESSES_size,
struct process** into_CPU_switch, struct process** outof_CPU_switch, int context_switch_time, int time_slice, int * time_slice_rem, int time_slice_start, char* alg)
{
	if (*CPU_BURST_PROCESS != NULL)
	{
		//total(original) CPU Burst time - (current time - CPU arrival time)
		//(*CPU_BURST_PROCESS)->CPU_remaining_time = (*CPU_BURST_PROCESS)->CPU_burst_times[(*CPU_BURST_PROCESS)->curr_CPU_index] - (time-((*CPU_BURST_PROCESS)->curr_CPU_arrival_time));
		(*CPU_BURST_PROCESS)->CPU_remaining_time = (*CPU_BURST_PROCESS)->curr_CPU_initial_rem_time - (time-((*CPU_BURST_PROCESS)->curr_CPU_arrival_time));
		#if DEBUG_MODE
		printf("The latest remainder time when it first got to the CPU was %d\n", (*CPU_BURST_PROCESS)->curr_CPU_initial_rem_time);
		printf("CPU PROCESS %c BURST PROCESS HAS %d REMAINING TIME\n", (*CPU_BURST_PROCESS)->id, (*CPU_BURST_PROCESS)->CPU_remaining_time);
		#endif
	}
	if (IO_PROCESSES_size > 0)
	{
		for (int i = 0; i < IO_PROCESSES_size; i++)
		{
		//total(original) CPU Burst time - (current time - CPU arrival time)
		(*IO_PROCESSES)[i]->IO_remaining_time = ((*IO_PROCESSES)[i]->IO_burst_times[(*IO_PROCESSES)[i]->curr_IO_index]) - (time-((*IO_PROCESSES)[i]->curr_IO_arrival_time));
		}
	}
	if (*into_CPU_switch!=NULL)
	{
		(*into_CPU_switch)->switch_remaining_time = (context_switch_time/2) - (time - ((*into_CPU_switch)->switch_start_time));
	}
	if (*outof_CPU_switch!=NULL)
	{
		(*outof_CPU_switch)->switch_remaining_time = (context_switch_time/2) - (time - ((*outof_CPU_switch)->switch_start_time));
	}
	if (strcmp(alg, "RR") == 0)
	{
		if (*CPU_BURST_PROCESS != NULL)
		{
			(*time_slice_rem) = time_slice - (time - time_slice_start);
		}
	}
	if (strcmp(alg, "SRT") == 0)
	{
		if (*CPU_BURST_PROCESS != NULL)
		{
			(*CPU_BURST_PROCESS)->tau_remaining = (*CPU_BURST_PROCESS)->initial_tau_remaining - (time-(*CPU_BURST_PROCESS)->initial_tau_calculation_time);
			//printf("Time %d: Process %c got here at %d with a tau of %d, and now has tau remaining %d\n", time, (*CPU_BURST_PROCESS)->id, (*CPU_BURST_PROCESS)->curr_CPU_arrival_time, (*CPU_BURST_PROCESS)->initial_tau_remaining, (*CPU_BURST_PROCESS)->tau_remaining);
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
			#ifdef DEBUG_MODE
			printf("Comparing process %c (with remaining time %d) with process %c (with remaining time %d)\n", 
				(*least_rem_IO_time_Process)->id, (*least_rem_IO_time_Process)->IO_remaining_time, (*IO_PROCESSES)[i]->id, (*IO_PROCESSES)[i]->IO_remaining_time);
			#endif
			if (min_IO_time_left == ((*IO_PROCESSES)[i]->IO_remaining_time))
			{
				if ((*least_rem_IO_time_Process)->id > (*IO_PROCESSES)[i]->id)
				{
					(*least_rem_IO_time_Process) = (*IO_PROCESSES)[i];
				}
			}
			else if (min_IO_time_left > ((*IO_PROCESSES)[i]->IO_remaining_time))
			{
				//now least_rem_IO_time_Process should point to same struct as (IO_PROCESSES)[i] pointer
				(*least_rem_IO_time_Process) = (*IO_PROCESSES)[i];
				min_IO_time_left = (*IO_PROCESSES)[i]->IO_remaining_time;
			}
		}
		#ifdef DEBUG_MODE
		printf("next IO process to finish will be %c\n", (*least_rem_IO_time_Process)->id);
		#endif
	}
	else
	{
		(*least_rem_IO_time_Process) = NULL;
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
		printf("[Q");
		for (int i = 0; i < READY_QUEUE_size; i++)
		{
			printf(" %c", (*READY_QUEUE)[i]->id);
		}
		printf("]\n");
	}
}


/*used for debugging processes*/
void print_all_IO(int IO_PROCESSES_size, struct process*** IO_PROCESSES, int time)
{
	if (IO_PROCESSES_size == 0) 
	{
		printf("[IO <empty>]\n");
	}
	else
	{
		printf("[IO: ");
		for (int i = 0; i < IO_PROCESSES_size; i++)
		{
			printf("|PROCESS %c -> finish time %d|", (*IO_PROCESSES)[i]->id,((*IO_PROCESSES)[i]->IO_remaining_time) + time);
		}
		printf("]\n");
	}
}

void all_algorithm(int num_processes, double lambda, int seed, int upper_bound, int context_switch_time, int time_slice, float alpha, char* RRadd, char* alg, FILE* summary)
{
//for each process, get 4 random uniform distribution numbers from .1 to 1.0
	//To get process arrival time, get first random number, plug it into exp-random.c
	//To get number of CPU bursts, the second uniform distibution number is multiplied y 100, truncated, and incremented by 1
	//For each of these cpu bursts, identify the actual CPU burst time and the I/O burst time as the next two random numbers in the sequence; 
	//For the last CPU burst, do not generate an I/O burst time
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
		printf("Process %c [NEW] (arrival time %d ms) %d CPU bursts\n", all_processes[i].id, all_processes[i].arrival_time, all_processes[i].num_CPU_bursts);
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
	struct process* into_CPU_switch = NULL;
	struct process* outof_CPU_switch = NULL;
		
	/* following print has to be done before every algorithm*/
	for (int i = 0; i < num_processes; i++)
	{
		if (all_processes[i].num_CPU_bursts == 1)
		{
			printf("Process %c [NEW] (arrival time %d ms) %d CPU burst\n", all_processes[i].id, all_processes[i].arrival_time, all_processes[i].num_CPU_bursts);
		}
		else
		{
			printf("Process %c [NEW] (arrival time %d ms) %d CPU bursts\n", all_processes[i].id, all_processes[i].arrival_time, all_processes[i].num_CPU_bursts);
		}
	}
	
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
	//int queue_index = 0;
	//int test_int = 0;
	int* next_event_array = NULL;
	int time_slice_rem = time_slice;
	int time_slice_start;
	int print_time_limit = 999;
	int context_switch_count = 0;
	int num_preemptions = 0;
	int from_IO_to_preempt = 0;
	
	printf("time %dms: Simulator started for %s ", time, alg);
	print_ready_queue(READY_QUEUE_size, &READY_QUEUE);
	while (!finished)
	//while (test_int != 33)
	//while (time < 6242)
	{
		//test_int++;
		/*initialize event_array to be all zeroes*/
		#ifdef DEBUG_MODE
		printf("\n\n");
		#endif
		if (next_event_array == NULL)
		{
			next_event_array = calloc(6, sizeof(int));
		}
		else
		{
			next_event_array = realloc(next_event_array, 6 * sizeof(int));
			for (int i = 0; i < 6; i++)
			{
				next_event_array[i] = 0;
			}
		}
		//get_next_event(CPU_BURST_PROCESS, least_rem_IO_time_Process, all_processes, next_arrival_index, num_processes, &next_event_array, time);
		get_next_event(CPU_BURST_PROCESS, least_rem_IO_time_Process, all_processes, outof_CPU_switch, into_CPU_switch, next_arrival_index, num_processes, &next_event_array, time, time_slice_rem, alg);
		#ifdef DEBUG_MODE
		printf("next_event_array is [%d, %d, %d, %d, %d, %d]\n", next_event_array[0],next_event_array[1], next_event_array[2], next_event_array[3], next_event_array[4], next_event_array[5]);
		#endif
		
		/*next event is context switch in to CPU finishes */
		if (next_event_array[3] == 1)
		{
			context_switch_count += 1;
			CPU_BURST_PROCESS = into_CPU_switch;
			//CPU_BURST_PROCESS->switch_start_time = time;
			//Advance_Ready_Queue(&READY_QUEUE, &READY_QUEUE_size);
			time += CPU_BURST_PROCESS->switch_remaining_time;
			CPU_BURST_PROCESS->curr_CPU_arrival_time = time;
			CPU_BURST_PROCESS->curr_CPU_initial_rem_time = CPU_BURST_PROCESS->CPU_remaining_time;
			CPU_BURST_PROCESS->initial_tau_remaining = CPU_BURST_PROCESS->tau_remaining;
			CPU_BURST_PROCESS->initial_tau_calculation_time = time;
			//Advance_Ready_Queue(&READY_QUEUE, &READY_QUEUE_size);
			if (time <= print_time_limit)
			{
				if (CPU_BURST_PROCESS->CPU_remaining_time == CPU_BURST_PROCESS->CPU_burst_times[CPU_BURST_PROCESS->curr_CPU_index])
				{
					printf("time %dms: Process %c started using the CPU for %dms burst ", time, CPU_BURST_PROCESS->id, CPU_BURST_PROCESS->CPU_remaining_time);
					print_ready_queue(READY_QUEUE_size, &READY_QUEUE);
				}
				else
				{
					printf("time %dms: Process %c started using the CPU with %dms remaining ", time, CPU_BURST_PROCESS->id, CPU_BURST_PROCESS->CPU_remaining_time);
					print_ready_queue(READY_QUEUE_size, &READY_QUEUE);
				}
			}
			into_CPU_switch = NULL;
			time_slice_rem = time_slice;
			time_slice_start = time;
			#ifdef DEBUG_MODE
			printf("TIME SLICE OF %d STARTS AT %d AND WILL END AT %d\n", time_slice, time_slice_start, time+time_slice_rem);
			#endif
			update_remaining_times(&CPU_BURST_PROCESS, &IO_PROCESSES, time, IO_PROCESSES_size, &into_CPU_switch, &outof_CPU_switch, context_switch_time, time_slice, &time_slice_rem, time_slice_start, alg);
			//printf("1HELLO\n");
		}
		/*next event is context switch out of CPU finishes */
		if (next_event_array[4] == 1)
		{
			/*if CPU burst finished, take it out of CPU and put it into IO*/
			if (CPU_BURST_PROCESS->CPU_remaining_time <= 0)
			{
				#ifdef DEBUG_MODE
				printf("--time, %d: Now switching Process %c out of CPU\n", time, CPU_BURST_PROCESS->id);
				#endif
				if (CPU_BURST_PROCESS->num_CPU_bursts_remaining > 0 )
				{
					CPU_BURST_PROCESS->curr_CPU_index += 1;
				}
				CPU_BURST_PROCESS->CPU_remaining_time = CPU_BURST_PROCESS->CPU_burst_times[CPU_BURST_PROCESS->curr_CPU_index];
				//time += (context_switch_time)/2;
				time += CPU_BURST_PROCESS->switch_remaining_time;
				
				if (CPU_BURST_PROCESS->num_CPU_bursts_remaining > 0 )
				{
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
				}
				else
				{
					CPU_BURST_PROCESS->terminated = 1;
				}
			}
			/*if CPU burst hasnt finished(it was preempted), take it out of the CPU and put it into the Ready Queue*/
			else
			{
				num_preemptions+=1;
				time += CPU_BURST_PROCESS->switch_remaining_time;
				//queue_index = Add_to_Ready_Queue(&READY_QUEUE, &READY_QUEUE_size, RRadd, CPU_BURST_PROCESS);
				Add_to_Ready_Queue(&READY_QUEUE, &READY_QUEUE_size, "END", CPU_BURST_PROCESS, alg);
				CPU_BURST_PROCESS->ready_queue_arrival_time = time;
			}
			#ifdef DEBUG_MODE
			printf("--time, %d: Switched Process %c out of CPU\n", time, CPU_BURST_PROCESS->id);
			#endif
			CPU_BURST_PROCESS = NULL;
			outof_CPU_switch = NULL;
			update_remaining_times(&CPU_BURST_PROCESS, &IO_PROCESSES, time, IO_PROCESSES_size, &into_CPU_switch, &outof_CPU_switch, context_switch_time, time_slice, &time_slice_rem, time_slice_start, alg);
		}
		/*next event is time slice expires*/
		if (next_event_array[5] == 1)
		{
			CPU_BURST_PROCESS->CPU_remaining_time -= time_slice_rem;
			time += time_slice_rem;
			/*if there is a process in the ready queue*/
			if (READY_QUEUE_size > 0)
			{
				if (time <= print_time_limit)
				{
					printf("time %dms: Time slice expired; process %c preempted with %dms to go ", time, CPU_BURST_PROCESS->id ,CPU_BURST_PROCESS->CPU_remaining_time);
					print_ready_queue(READY_QUEUE_size, &READY_QUEUE);
				}
				CPU_BURST_PROCESS->switch_start_time = time;
				outof_CPU_switch = CPU_BURST_PROCESS;
			}
			else
			{
				if (time <= print_time_limit)
				{
					printf("time %dms: Time slice expired; no preemption because ready queue is empty ", time);
					print_ready_queue(READY_QUEUE_size, &READY_QUEUE);
				}
				//CPU_BURST_PROCESS->switch_start_time = time;
				//Advance_Ready_Queue(&READY_QUEUE, &READY_QUEUE_size);
				CPU_BURST_PROCESS->curr_CPU_arrival_time = time;
				CPU_BURST_PROCESS->curr_CPU_initial_rem_time = CPU_BURST_PROCESS->CPU_remaining_time;
				time_slice_rem = time_slice;
				time_slice_start = time;
				#if DEBUG_MODE
				printf("1Time_slice_rem is %d\n", time_slice_rem);
				#endif
			}
			
			update_remaining_times(&CPU_BURST_PROCESS, &IO_PROCESSES, time, IO_PROCESSES_size, &into_CPU_switch, &outof_CPU_switch, context_switch_time, time_slice, &time_slice_rem, time_slice_start, alg);
		}
		/*next event is CPU burst finishes*/
		if (next_event_array[0] == 1)
		{
			/*increase time by the remaining burst time and 
			bring CPU process remaining time down to 0*/
			time += CPU_BURST_PROCESS->CPU_remaining_time;
			CPU_BURST_PROCESS->CPU_remaining_time -= CPU_BURST_PROCESS->CPU_remaining_time;
			CPU_BURST_PROCESS->num_CPU_bursts_remaining -= 1;
			CPU_BURST_PROCESS->turnaround_time += (time - CPU_BURST_PROCESS->curr_CPU_first_arrival_time) + context_switch_time/2;
			#ifdef DEBUG_MODE
			printf("Process %c burst arrived at time %d\n", CPU_BURST_PROCESS->id, CPU_BURST_PROCESS->curr_CPU_first_arrival_time);
			printf("Turnaround time for process %c is %d\n", CPU_BURST_PROCESS->id, CPU_BURST_PROCESS->turnaround_time);
			#endif

			/*increment CPU burst index and set remaining time to new corresponding CPU burst*/
			/*only do this if there is a next CPU burst*/
			/*also, if not last burst, process does IO burst, 
				which is denoted by it going into I/O processes array*/
			if (CPU_BURST_PROCESS->num_CPU_bursts_remaining > 0 )
			{		
				if (time <= print_time_limit)
				{
					if (CPU_BURST_PROCESS->num_CPU_bursts_remaining > 1)
					{
						printf("time %dms: Process %c completed a CPU burst; %d bursts to go ", time, CPU_BURST_PROCESS->id, CPU_BURST_PROCESS->num_CPU_bursts_remaining);
					}
					else
					{
						printf("time %dms: Process %c completed a CPU burst; %d burst to go ", time, CPU_BURST_PROCESS->id, CPU_BURST_PROCESS->num_CPU_bursts_remaining);
					}
					
					print_ready_queue(READY_QUEUE_size, &READY_QUEUE);
					/*
					if (strcmp(alg, "SJF") == 0 || strcmp(alg, "SRT") == 0)
					{
						CPU_BURST_PROCESS->tau = (int)(ceil(alpha * CPU_BURST_PROCESS->CPU_burst_times[CPU_BURST_PROCESS->curr_CPU_index] + alpha * CPU_BURST_PROCESS->tau));
						printf("time %dms: Recalculated tau = %dms for process %c ", time, CPU_BURST_PROCESS->tau, CPU_BURST_PROCESS->id);
						print_ready_queue(READY_QUEUE_size, &READY_QUEUE);
					}
					*/
				}
				if (strcmp(alg, "SJF") == 0 || strcmp(alg, "SRT") == 0)
				{
					CPU_BURST_PROCESS->tau = (int)(ceil(alpha * CPU_BURST_PROCESS->CPU_burst_times[CPU_BURST_PROCESS->curr_CPU_index] + (1.0-alpha) * CPU_BURST_PROCESS->tau));
					CPU_BURST_PROCESS->tau_remaining = CPU_BURST_PROCESS->tau;
					CPU_BURST_PROCESS->initial_tau_remaining = CPU_BURST_PROCESS->tau;
					CPU_BURST_PROCESS->initial_tau_calculation_time = time;
					if (time <= print_time_limit)
					{	
						printf("time %dms: Recalculated tau = %dms for process %c ", time, CPU_BURST_PROCESS->tau, CPU_BURST_PROCESS->id);
						print_ready_queue(READY_QUEUE_size, &READY_QUEUE);
					}
				}
				CPU_BURST_PROCESS->curr_IO_index = CPU_BURST_PROCESS->curr_CPU_index;
				CPU_BURST_PROCESS->IO_remaining_time = CPU_BURST_PROCESS->IO_burst_times[CPU_BURST_PROCESS->curr_IO_index];
				
				if (time <= print_time_limit)
				{
					printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms ", 
						time, CPU_BURST_PROCESS->id, (time + (context_switch_time/2) + CPU_BURST_PROCESS->IO_remaining_time));
					print_ready_queue(READY_QUEUE_size, &READY_QUEUE);
				}
				
				CPU_BURST_PROCESS->switch_start_time = time;
				outof_CPU_switch = CPU_BURST_PROCESS;
				update_remaining_times(&CPU_BURST_PROCESS, &IO_PROCESSES, time, IO_PROCESSES_size, &into_CPU_switch, &outof_CPU_switch, context_switch_time, time_slice, &time_slice_rem, time_slice_start, alg);
			}
			else
			{
				printf("time %dms: Process %c terminated ", time, CPU_BURST_PROCESS->id);
				print_ready_queue(READY_QUEUE_size, &READY_QUEUE);
				CPU_BURST_PROCESS->switch_start_time = time;
				outof_CPU_switch = CPU_BURST_PROCESS;
			}

		}
		//finished = 1;
		/*next event is PROCESS I/O FINISHES*/
		if (next_event_array[1] == 1)
		{
			//put it on to the ready queue. Logic for putting it into the queue will change in each alg.
			time += least_rem_IO_time_Process->IO_remaining_time;

			Add_to_Ready_Queue(&READY_QUEUE, &READY_QUEUE_size, RRadd, least_rem_IO_time_Process, alg);
			if (least_rem_IO_time_Process->CPU_remaining_time == least_rem_IO_time_Process->CPU_burst_times[least_rem_IO_time_Process->curr_CPU_index])
			{
				least_rem_IO_time_Process->curr_CPU_first_arrival_time = time;
			}	
			least_rem_IO_time_Process->ready_queue_arrival_time = time;
			//find and take process out of IO array
			for (int i = 0; i < IO_PROCESSES_size; i++)
			{
				if (least_rem_IO_time_Process->id == IO_PROCESSES[i]->id)
				{
					for (int j = i; j < (IO_PROCESSES_size-1); j++)
					{
						(IO_PROCESSES)[j] = (IO_PROCESSES)[j+1];
					}
					(IO_PROCESSES_size) -= 1;
					(IO_PROCESSES) = realloc(IO_PROCESSES, IO_PROCESSES_size*sizeof(struct process *));
					break;
				}
			}
			/*
			if (strcmp("SRT", alg) == 1)
			{
				if (time <= print_time_limit)
					printf("time %dms: Process %c completed I/O; added to ready queue ", time, least_rem_IO_time_Process->id);
			}
			*/
			if (strcmp("SRT", alg) == 0)
			{
				/*if, from IO, the process was inserted into the first slot of the ready queue and there is a process in the CPU*/
				if ((CPU_BURST_PROCESS != NULL) && (into_CPU_switch == NULL) && (outof_CPU_switch == NULL) && (least_rem_IO_time_Process->id == READY_QUEUE[0]->id))
				{
					update_remaining_times(&CPU_BURST_PROCESS, &IO_PROCESSES, time, IO_PROCESSES_size, &into_CPU_switch, &outof_CPU_switch, 
						context_switch_time, time_slice, &time_slice_rem, time_slice_start, alg);
					if (READY_QUEUE[0]->tau_remaining < CPU_BURST_PROCESS->tau_remaining)
					{
						from_IO_to_preempt = 1;
						/*if (time <= print_time_limit)
						{
							printf("time %dms: Process %c (tau %d) completed I/O and will preempt %c ", time, READY_QUEUE[0]->id, READY_QUEUE[0]->tau, CPU_BURST_PROCESS->id);
						}				
						CPU_BURST_PROCESS->switch_start_time = time;
						outof_CPU_switch = CPU_BURST_PROCESS;*/
					}
				}
			}
			if (from_IO_to_preempt == 0)
			{
				if (time <= print_time_limit)
				{
					if (strcmp("SRT", alg) != 0)
					{
						printf("time %dms: Process %c completed I/O; added to ready queue ", time, least_rem_IO_time_Process->id);
					}
					else if (strcmp("SRT", alg) == 0)
					{
						printf("time %dms: Process %c (tau %dms) completed I/O; added to ready queue ", time, least_rem_IO_time_Process->id, least_rem_IO_time_Process->tau_remaining);
					}
					print_ready_queue(READY_QUEUE_size, &READY_QUEUE);
				}
			}
			update_remaining_times(&CPU_BURST_PROCESS, &IO_PROCESSES, time, IO_PROCESSES_size, &into_CPU_switch, &outof_CPU_switch, context_switch_time, time_slice, &time_slice_rem, time_slice_start, alg);
		}
		/*next event is process arrival*/
		if (next_event_array[2] == 1)
		{
			//put it on to the ready queue. Logic for putting it into the queue will change in each alg.
			//queue_index = Add_to_Ready_Queue(&READY_QUEUE, &READY_QUEUE_size, RRadd, &all_processes[next_arrival_index]);
			Add_to_Ready_Queue(&READY_QUEUE, &READY_QUEUE_size, RRadd, &all_processes[next_arrival_index], alg);
			all_processes[next_arrival_index].curr_CPU_index = 0;
			all_processes[next_arrival_index].CPU_remaining_time = all_processes[next_arrival_index].CPU_burst_times[0];
			time = (all_processes[next_arrival_index].arrival_time);
			all_processes[next_arrival_index].curr_CPU_first_arrival_time = time;
			all_processes[next_arrival_index].ready_queue_arrival_time = time;
			if (time <= print_time_limit)
			{
				if (strcmp(alg, "SJF") == 0 || strcmp(alg, "SRT") == 0)
				{
					printf("time %dms: Process %c (tau %dms) arrived; added to ready queue ", time, all_processes[next_arrival_index].id, all_processes[next_arrival_index].tau_remaining);
				}
				else
				{
					printf("time %dms: Process %c arrived; added to ready queue ", time, all_processes[next_arrival_index].id);
				}
				print_ready_queue(READY_QUEUE_size, &READY_QUEUE);
			}
			next_arrival_index += 1;

		}
		//if new process in front of ready queue and CPU is free, put process into CPU and take it out of the ready queue 
		if ((READY_QUEUE_size > 0) && (CPU_BURST_PROCESS == NULL) && (into_CPU_switch == NULL))
		{
			//printf("2HELLO\n");
			into_CPU_switch = READY_QUEUE[0];
			into_CPU_switch->switch_start_time = time;
			into_CPU_switch->wait_time += (time - into_CPU_switch->ready_queue_arrival_time);
			//into_CPU_switch->turnaround_time += (time - into_CPU_switch->ready_queue_arrival_time);
			Advance_Ready_Queue(&READY_QUEUE, &READY_QUEUE_size);
			#ifdef DEBUG_MODE
			printf("3Process %c is switching into the cpu\n", into_CPU_switch->id);
			#endif
		}
		//handles case in SRT when new process is added to front of queue and 
		//  may cause a preemption if its tau is less than remaining tme of process in CPU
		else if ((strcmp(alg, "SRT") == 0) && (READY_QUEUE_size > 0) && (CPU_BURST_PROCESS != NULL) && (into_CPU_switch == NULL) && (outof_CPU_switch == NULL))
		{
			//printf("3HELLO\n");
			update_remaining_times(&CPU_BURST_PROCESS, &IO_PROCESSES, time, IO_PROCESSES_size, &into_CPU_switch, &outof_CPU_switch, 
				context_switch_time, time_slice, &time_slice_rem, time_slice_start, alg);
			if (READY_QUEUE[0]->tau_remaining < CPU_BURST_PROCESS->tau_remaining)
			{
				if (from_IO_to_preempt == 0)
				{
					if (time <= print_time_limit)
					{
						printf("time %dms: Process %c (tau %dms) will preempt %c ", time, READY_QUEUE[0]->id, READY_QUEUE[0]->tau_remaining, CPU_BURST_PROCESS->id);
						print_ready_queue(READY_QUEUE_size, &READY_QUEUE);
					}
				}
				else if (from_IO_to_preempt == 1)
				{
					if (time <= print_time_limit)
					{
						printf("time %dms: Process %c (tau %dms) completed I/O and will preempt %c ", time, READY_QUEUE[0]->id, READY_QUEUE[0]->tau_remaining, CPU_BURST_PROCESS->id);
						print_ready_queue(READY_QUEUE_size, &READY_QUEUE);
					}
					from_IO_to_preempt = 0;
				}	
				CPU_BURST_PROCESS->switch_start_time = time;
				outof_CPU_switch = CPU_BURST_PROCESS;
			}
		}
		
		//update_remaining_times(&CPU_BURST_PROCESS, &IO_PROCESSES, time, IO_PROCESSES_size);	
		update_remaining_times(&CPU_BURST_PROCESS, &IO_PROCESSES, time, IO_PROCESSES_size, &into_CPU_switch, &outof_CPU_switch, context_switch_time, time_slice, &time_slice_rem, time_slice_start, alg);
		#ifdef DEBUG_MODE	
		print_all_IO(IO_PROCESSES_size, &IO_PROCESSES, time);
		#endif
		update_next_IO_finish(&least_rem_IO_time_Process, &IO_PROCESSES, IO_PROCESSES_size);
		#ifdef DEBUG_MODE
		if (into_CPU_switch != NULL)
		{
			if (time <= print_time_limit)
				printf("4Process %c is switching into the cpu and has rem time %d \n", into_CPU_switch->id, into_CPU_switch->switch_remaining_time);
		}
		#endif
		finished = 1;
		for (int i = 0; i < num_processes; i++)
		{
			if (all_processes[i].terminated == 0)
			{
				finished = 0;
			}
		}
	}
	printf("time %dms: Simulator ended for %s ", time, alg);
	print_ready_queue(READY_QUEUE_size, &READY_QUEUE);
	
	fprintf(summary, "Algorithm %s\n", alg);
	
	/*CPU BURST AVERAGE CALCULATION*/
	int CPU_burst_times_total = 0;
	int CPU_burst_num_total = 0;
	for (int i = 0; i < num_processes; i++)
	{
		CPU_burst_num_total += all_processes[i].num_CPU_bursts;
		for (int j = 0; j < all_processes[i].num_CPU_bursts; j++)
		{
			CPU_burst_times_total += all_processes[i].CPU_burst_times[j];
		}
	}	
	float av_CPU_burst_times = CPU_burst_times_total / (CPU_burst_num_total * 1.0);		
	fprintf(summary, "-- average CPU burst time: %.3f ms\n", av_CPU_burst_times);
	
	/*WAIT TIME AVERAGE CALCULATION*/
	int wait_time_total = 0;
	for (int i = 0; i < num_processes; i++)
	{
		wait_time_total += all_processes[i].wait_time;
	}
	float av_wait_time = wait_time_total/(CPU_burst_num_total*1.0);
	fprintf(summary, "-- average wait time: %.3f ms\n", av_wait_time);

	/*TURN AROUND TIME AVERAGE CALCULATION*/
	int turnaround_time_total;
	for (int i = 0; i < num_processes; i++)
	{
		turnaround_time_total += all_processes[i].turnaround_time;
	}
	float av_turnaround_time = turnaround_time_total / (CPU_burst_num_total*1.0);
	fprintf(summary, "-- average turnaround time: %.3f ms\n", av_turnaround_time);
	
	/*TOTAL NUMBER OF CONTEXT SWITCHES*/
	fprintf(summary, "-- total number of context switches: %d\n", context_switch_count);
	
	/*TOTAL NUMBER OF PREEMPTIONS*/
	fprintf(summary, "-- total number of preemptions: %d\n", num_preemptions);
	
	free(next_event_array);
	for (int i = 0; i < num_processes; i++)
	{
		free(all_processes[i].CPU_burst_times);
		free(all_processes[i].IO_burst_times);
	}
	free(all_processes);
	free(READY_QUEUE);
	free(IO_PROCESSES);
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
	float alpha = atof(argv[6]);
	int time_slice = atoi(argv[7]);
	

	FILE *summary;
	summary = fopen("simout.txt", "w");
	
	all_algorithm(num_processes, lambda, seed, upper_bound, context_switch_time, time_slice, alpha, "END", "SJF", summary);
	printf("\n");
	
	all_algorithm(num_processes, lambda, seed, upper_bound, context_switch_time, time_slice, alpha, "END", "SRT", summary);
	printf("\n");
	
	all_algorithm(num_processes, lambda, seed, upper_bound, context_switch_time, time_slice, alpha, "END", "FCFS", summary);
	printf("\n");
	
	all_algorithm(num_processes, lambda, seed, upper_bound, context_switch_time, time_slice, alpha, RRadd, "RR", summary);
	
	fclose(summary);
	
	return EXIT_SUCCESS;
}
