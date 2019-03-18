struct process
{
	char id;
	int num_CPU_bursts;
	int* CPU_burst_times;
	int* IO_burst_times;
	int arrival_time;
	
	int wait_time;
	int turnaround_time;
	int num_CPU_bursts_remaining;
	
	//will hold the index that denotes current IO burst from IO_burst_times
	int curr_IO_index;
	//will hold remaining time for current CPU burst
	int IO_remaining_time;
	//will hold the index that denotes current CPU burst from CPU_burst_times
	int curr_CPU_index;
	//will hold remaining time for current CPU burst
	int CPU_remaining_time;
	int switch_remaining_time;
	int switch_start_time;
	
	//at what time did process enter CPU or I/O
	int curr_CPU_arrival_time;
	int curr_IO_arrival_time;
	
	/*if 0, process is not terminated. If 1, process has terminated*/
	int terminated ;
	
};

/*https://stackoverflow.com/questions/6105513/need-help-using-qsort-with-an-array-of-structs*/
int compare_arrival_time(const void *p1, const void *p2)
{
   const struct process *elem1 = p1;    
   const struct process *elem2 = p2;
   
   if (elem1->arrival_time < elem2->arrival_time)
      return -1;
   else if (elem1->arrival_time > elem2->arrival_time)
      return 1;
   /*if times are the same, sort by alphabetical order*/
   else if (elem1->arrival_time == elem2->arrival_time)
   {
  		if (elem1->id < elem2->id)
  			return -1;
  		else
  			return 1;
   }
   else
      return 0;
}
