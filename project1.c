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
	
	/* FIRST COME FIRST SERVE */////////////////
	READY_QUEUE
}




