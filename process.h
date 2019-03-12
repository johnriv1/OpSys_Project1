struct process
{
	char id;
	int num_CPU_bursts;
	int* CPU_burst_times;
	int* IO_burst_times;
	int arrival_time;
};
