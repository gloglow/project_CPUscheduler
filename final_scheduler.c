#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct process {					// process structure
	int ID, CPUburst, IOburst, cycle, arrivaltime, priority, finish, current_CPU, current_IO, current_cycle, tatime, wttime, tq, current_wt;
}process;					
/* 
	ID:process num
	CPUburst:cpu burst per cycle
	IOburst:io burst per cycle
	cycle:the number of cycle
	arrivaltime
	priority:the lower number, the higher priority
	finish:default -> 0, finish -> 1, 
	current_CPU:left CPU burst
	current_IO:left IO burst
	current_cycle:left cycle
	tatime:turnaround time
	wttime:waiting time
	tq:current time quantum
	current_wt:counter for aging. if > 20, priority minus 1.
*/

typedef struct queue {					// ready queue, waiting queue structure			
	struct process *data[10];				
	int in, out, size;					
}queue;
/*
	data:pointer of process in job queue
	in:enqueue 
	out:dequeue
	size:the number of processes in queue
*/

process create();					// creating processes
void FCFS();					// FCFS simul
void SJF();					// SJF simul
void Priority();					// Priority alg simul
void PRE_SJF();
void PRE_Priority();
void RR();
void alg_analyze();					// evaluation

void en_readyqueue();				
void en_waitingqueue();
void de_readyqueue();
void de_waitingququq();
int empty_readyqueue();
int empty_waitingqueue();
void arrive_readyqueue();
void initializing();
void IO_bursting();
void finish();
void shortest_job_sorting();
void priority_sorting();

int systemtime;					
int the_num_process;					// the number of processes
int time_quantum;
int aging_or_not;

process job_queue[10];					
queue ready_queue;					
queue waiting_queue;					
process* CPU_use;					// pointer of process which is using cpu
process* IO_use;					// pointer of process which is using io

float FCFS_TA=0;					// turnaround time
float SJF_TA=0;
float Priority_TA=0;
float PRE_SJF_TA = 0;
float PRE_Priority_TA = 0;
float RR_TA = 0;

float FCFS_WT = 0;					// waiting time
float SJF_WT = 0;
float Priority_WT = 0;
float PRE_SJF_WT = 0;
float PRE_Priority_WT = 0;
float RR_WT = 0;

float FCFS_CPU_utilization;					// cpu utilization
float SJF_CPU_utilization;
float Priority_CPU_utilization;
float PRE_SJF_CPU_utilization;
float PRE_Priority_CPU_utilization;
float RR_CPU_utilization;

int main()
{
	int i;
	printf("Insert the number of processes (maximum 9)\n");
	scanf("%d", &the_num_process);
	printf("Insert time quantum for Round Robin.");
	scanf("%d", &time_quantum);
	printf("aging in Priority alg? (YES-1 NO-0)");
	scanf("%d", &aging_or_not);

	printf("\nCreating %d processes.\n\n", the_num_process);

	for (i = 0; i < the_num_process; i++)
	{
		srand((unsigned)time(NULL)-i);					// random assign
		job_queue[i] = create(i);					
	}

	printf("~ PROCESSES ~ (the lower number, the higher priority)");
	for (i = 0; i < the_num_process; i++)
	{
		printf("\nProcess ID:%d CPU burst:%d I/O burst:%d cycle:%d arrival time:%d priority:%d\n\n", job_queue[i].ID,  job_queue[i].CPUburst, job_queue[i].IOburst, job_queue[i].cycle, job_queue[i].arrivaltime, job_queue[i].priority);
	}					

	printf("\n\nFirst Come First Served");
	FCFS();

	printf("\n\nShortest Job First");
	SJF();

	printf("\n\nPriority");
	Priority();

	printf("\n\nRound Robin");
	RR();

	printf("\n\nPreemptive SJF");
	PRE_SJF();

	printf("\n\nPreemptive Priority");
	PRE_Priority();

	printf("\n\n<< evaluation >>");
	alg_analyze();

	return 0;
}

process create(int i)
{
	int pID, pCPUburst, pIOburst, pcycle, parrivaltime, ppriority;
	
	pID = i + 1;
	pCPUburst = (rand() % 5) + 1;
	pIOburst = (rand() % 5);

	if (pIOburst == 0)
		pcycle = 1;
	else
		pcycle = (rand() % 3)+2;
	parrivaltime = (rand() % 10);
	ppriority = (rand() % 10);
	
	process createdprocess = { pID, pCPUburst, pIOburst, pcycle, parrivaltime, ppriority, 0, pCPUburst, pIOburst, pcycle, 0, 0, time_quantum, 0};
	return createdprocess;
}

void en_readyqueue(process* pprocess)					// insert
{
	ready_queue.data[ready_queue.in] = pprocess;
	ready_queue.in=(ready_queue.in+1)%(the_num_process+1);
	ready_queue.size++;
}

void en_waitingqueue(process* pprocess)
{
	waiting_queue.data[waiting_queue.in] = pprocess;
	waiting_queue.in = (waiting_queue.in + 1) % (the_num_process+1);
	waiting_queue.size++;
}

void de_readyqueue()					// delete
{
	CPU_use = ready_queue.data[ready_queue.out];
	ready_queue.data[ready_queue.out] = NULL;
	ready_queue.out = (ready_queue.out + 1) % (the_num_process+1);
	ready_queue.size--;
}

void de_waitingqueue()
{
	waiting_queue.data[waiting_queue.out] = NULL;
	waiting_queue.out = (waiting_queue.out + 1) % (the_num_process+1);
	waiting_queue.size--;
}

int empty_readyqueue()					// check queue is empty(return 1) or not(return 0). 
{
	if (ready_queue.in == ready_queue.out)
		return 1;
	else
		return 0;
}

int empty_waitingqueue()
{
	if (waiting_queue.in == waiting_queue.out)
		return 1;
	else
		return 0;
}

void arrive_readyqueue()					// insert process into ready queue if the arrival time is system time.
{
	int i;
	for (i = 0; i < the_num_process; i++)
	{
		if (job_queue[i].arrivaltime == systemtime)
			en_readyqueue(&job_queue[i]);
	}
	
	return;
}

void initializing()						// before simul, initializing data
{
	int i;
	systemtime = 0;
	CPU_use = NULL;
	IO_use = NULL;
	for (i = 0; i < the_num_process; i++)
	{
		job_queue[i].current_CPU = job_queue[i].CPUburst;
		job_queue[i].current_IO = job_queue[i].IOburst;
		job_queue[i].current_cycle = job_queue[i].cycle;
		job_queue[i].tq = time_quantum;
		job_queue[i].finish = 0;
		job_queue[i].tatime = 0;
		job_queue[i].wttime = 0;
		job_queue[i].current_wt = 0;
		ready_queue.data[i] = NULL;
		waiting_queue.data[i] = NULL;
	}
	ready_queue.data[i + 1] = NULL;
	waiting_queue.data[i + 1] = NULL;
	ready_queue.size = 0;
	waiting_queue.size = 0;
	ready_queue.in = 0;
	ready_queue.out = 0;
	waiting_queue.in = 0;
	waiting_queue.out = 0;
	return;
}

void cycle_update(process* pprocess)					// update cycle of processes
{
	if (pprocess->current_CPU == 0 && pprocess->current_IO == 0)
	{
		pprocess->current_cycle--;
		pprocess->current_CPU = pprocess->CPUburst;
		pprocess->current_IO = pprocess->IOburst;
		pprocess->tq = time_quantum;
	}
	return;
}

void IO_burst_ready()					// before io burst
{
	if (empty_waitingqueue() == 1)
	{
		IO_use = NULL;
		return;
	}
	IO_use = waiting_queue.data[waiting_queue.out];
	de_waitingqueue();
	return;
}

void IO_bursting()					
{
	if (IO_use == NULL)					// io ready
		IO_burst_ready();
	else
	{
		printf("   PROCESS   %d I/O OPERATION", IO_use->ID);
		IO_use->current_IO--;					// io bursting
		if (IO_use->current_IO == 0)					
		{
			cycle_update(IO_use);
			if (IO_use->current_cycle == 0)					
			{
				finish(IO_use);
			}
			else					
			{
				en_readyqueue(IO_use);
			}
			IO_burst_ready();
		}
	}
	return;
}

void finish(process* finished_process)
{
	printf("   PROCESS   %d finished.", finished_process->ID);
	finished_process->finish = 1;
}

void shortest_job_sorting()					// in sjf, sorting ready queue by cpu burst.
{
	int i;
	process* temp;
	int least = ready_queue.out;
	for (i = 0; i < the_num_process + 1; i++)
	{
		if (ready_queue.data[i] != NULL && ready_queue.data[i]->CPUburst < ready_queue.data[least]->CPUburst)
			least = i;
	}
	if (least != ready_queue.out)
	{
		temp = ready_queue.data[ready_queue.out];
		ready_queue.data[ready_queue.out] = ready_queue.data[least];
		ready_queue.data[least] = temp;
	}
}

void priority_sorting()					// in sjf, sorting ready queue by priority.
{
	int i;
	process* temp;
	int least = ready_queue.out;
	for (i = 0; i < the_num_process + 1; i++)
	{
		if (ready_queue.data[i] != NULL && ready_queue.data[i]->priority < ready_queue.data[least]->priority)
			least = i;
	}
	if (least != ready_queue.out)
	{
		temp = ready_queue.data[ready_queue.out];
		ready_queue.data[ready_queue.out] = ready_queue.data[least];
		ready_queue.data[least] = temp;
	}
}

void FCFS()
{
	int i;
	int count=0;
	initializing();
	FCFS_CPU_utilization = 0;
	while (count != the_num_process)
	{
		count = 0;
		arrive_readyqueue();
		printf("\n time= %d\n", systemtime);
		if (CPU_use != NULL)					// 1. there is a process using cpu -> the process cpu burst
		{
			CPU_use->current_CPU--;
			printf("	PROCESS   %d run", CPU_use->ID);
			if (CPU_use->current_CPU == 0)
			{
				cycle_update(CPU_use);					
				if (CPU_use->current_cycle == 0)					// if cycle of the process is 0, finished
				{
					finish(CPU_use);
					CPU_use = NULL;
				}
				else				// if cycle of the process is not 0,
				{
					if (CPU_use->IOburst == 0)					// io burst x -> ready queue
						en_readyqueue(CPU_use);
					else					// io burst o -> waiting queue
						en_waitingqueue(CPU_use);
					CPU_use = NULL;
				}
			}
		}
		else if (empty_readyqueue() != 1)	// 2. there isnt a process using cpu, and there is process in ready queue
		{
			de_readyqueue();
			CPU_use->current_CPU--;
			CPU_use->tq--;
			printf("	PROCESS   %d run", CPU_use->ID);
			if (CPU_use->current_CPU == 0)
			{
				cycle_update(CPU_use);					
				if (CPU_use->current_cycle ==0)					// if cycle of the process is 0, finished
				{
					finish(CPU_use);
					CPU_use = NULL;
				}
				else				// if cycle of the process is not 0,
				{
					if (CPU_use->IOburst == 0)					// io burst x -> ready queue
						en_readyqueue(CPU_use);
					else					// io burst o -> waiting queue
						en_waitingqueue(CPU_use);
					CPU_use = NULL;
				}
			}
		}
		else
		{
			printf("	IDLE");					// 3. ready queue is empty
			FCFS_CPU_utilization++;
		}

		IO_bursting();

		for (i = 0; i < the_num_process; i++)
		{
			if (job_queue[i].finish == 1)
				count++;
			else
				job_queue[i].tatime++;
			if (ready_queue.data[i] != NULL)
				ready_queue.data[i]->wttime++;
		}
		systemtime++;
	}
	printf("\n\nALL PROCESSES ARE FINISHED.");
	for (i = 0; i < the_num_process; i++)
	{
		FCFS_TA = FCFS_TA + (job_queue[i].tatime) - (job_queue[i].arrivaltime) + 1;
		FCFS_WT = FCFS_WT + job_queue[i].wttime;
	}
	FCFS_TA = FCFS_TA / the_num_process;
	FCFS_WT = FCFS_WT / the_num_process;
	FCFS_CPU_utilization = (systemtime - FCFS_CPU_utilization) / systemtime * 100;
	return;
}

void SJF()
{
	int i;
	int count = 0;
	initializing();
	SJF_CPU_utilization = 0;
	while (count != the_num_process)
	{
		count = 0;
		arrive_readyqueue();
		if (empty_readyqueue != 0)
			shortest_job_sorting();					// sorting processes in ready queue by cpu burst
		printf("\n time= %d\n", systemtime);
		if (CPU_use != NULL)					// 1. there is a process using cpu -> the process cpu burst
		{
			CPU_use->current_CPU--;
			printf("	PROCESS   %d run", CPU_use->ID);
			if (CPU_use->current_CPU == 0)					
			{
				cycle_update(CPU_use);					
				if (CPU_use->current_cycle == 0)					// if cycle of the process is 0, finished
				{
					finish(CPU_use);
					CPU_use = NULL;
				}
				else				// if cycle of the process is not 0,
				{
					if (CPU_use->IOburst == 0)					// io burst x -> ready queue
						en_readyqueue(CPU_use);
					else					// io burst o -> waiting queue
						en_waitingqueue(CPU_use);
					CPU_use = NULL;
				}
			}
		}
		else if (empty_readyqueue() != 1)	// 2. there isnt a process using cpu, and there is process in ready queue
		{
			de_readyqueue();
			CPU_use->current_CPU--;
			printf("	PROCESS   %d run", CPU_use->ID);
			if (CPU_use->current_CPU == 0)
			{
				cycle_update(CPU_use);					
				if (CPU_use->current_cycle == 0)					// if cycle of the process is 0, finished
				{
					finish(CPU_use);
					CPU_use = NULL;
				}
				else				// if cycle of the process is not 0,
				{
					if (CPU_use->IOburst == 0)					// io burst x -> ready queue
						en_readyqueue(CPU_use);
					else					// io burst o -> waiting queue
						en_waitingqueue(CPU_use);
					CPU_use = NULL;
				}
			}
		}	
		else
		{
			printf("	IDLE");							// 3. ready queue is empty
			SJF_CPU_utilization++;
		}

		IO_bursting();

		for (i = 0; i < the_num_process; i++)
		{
			if (job_queue[i].finish == 1)
				count++;
			else
				job_queue[i].tatime++;
			if (ready_queue.data[i] != NULL)
				ready_queue.data[i]->wttime++;
		}
		systemtime++;
	}
	printf("\n\nALL PROCESSES ARE FINISHED.");
	for (i = 0; i < the_num_process; i++)
	{
		SJF_TA = SJF_TA + (job_queue[i].tatime) - (job_queue[i].arrivaltime) + 1;
		SJF_WT = SJF_WT + job_queue[i].wttime;
	}
	SJF_TA = SJF_TA / the_num_process;
	SJF_WT = SJF_WT / the_num_process;
	SJF_CPU_utilization = (systemtime - SJF_CPU_utilization) / systemtime * 100;
	return;
}

void Priority()
{
	int i;
	int count = 0;
	initializing();
	Priority_CPU_utilization = 0;
	while (count != the_num_process)
	{
		count = 0;
		arrive_readyqueue();
		if (empty_readyqueue != 0)
			priority_sorting();					// sorting processes by priority
		printf("\ntime = %d\n", systemtime);
		if (CPU_use != NULL)					// 1. there is a process using cpu -> the process cpu burst
		{
			CPU_use->current_CPU--;
			printf("	PROCESS   %d run", CPU_use->ID);
			if (CPU_use->current_CPU == 0)
			{
				cycle_update(CPU_use);
				if (CPU_use->current_cycle == 0)					// if cycle of the process is 0, finished
				{
					finish(CPU_use);
					CPU_use = NULL;
				}
				else				// if cycle of the process is not 0,
				{
					if (CPU_use->IOburst == 0)					// io burst x -> ready queue
						en_readyqueue(CPU_use);
					else					// io burst o -> waiting queue
						en_waitingqueue(CPU_use);
					CPU_use = NULL;
				}
			}
		}
		else if (empty_readyqueue() != 1)	// 2. there isnt a process using cpu, and there is process in ready queue
		{
			de_readyqueue();
			CPU_use->current_CPU--;
			printf("	PROCESS   %d run", CPU_use->ID);
			if (CPU_use->current_CPU == 0)
			{
				cycle_update(CPU_use);
				if (CPU_use->current_cycle == 0)					// if cycle of the process is 0, finished
				{
					finish(CPU_use);
					CPU_use = NULL;
				}
				else				// if cycle of the process is not 0, 
				{
					if (CPU_use->IOburst == 0)					// io burst x -> ready queue
						en_readyqueue(CPU_use);
					else					// io burst o -> waiting queue
						en_waitingqueue(CPU_use);
					CPU_use = NULL;
				}
			}
		}
		else
		{
			printf("	IDLE");						// 3. ready queue is empty
			Priority_CPU_utilization++;
		}

		IO_bursting();

		for (i = 0; i < the_num_process; i++)
		{
			if (job_queue[i].finish == 1)
				count++;
			else
				job_queue[i].tatime++;
			if (ready_queue.data[i] != NULL)
			{
				if (aging_or_not == 1)					// for aging, record waiting time of process
					ready_queue.data[i]->current_wt++;
				ready_queue.data[i]->wttime++;
			}
			if (job_queue[i].current_wt > 20)					// per wt of process>20, priority --
			{
				job_queue[i].priority--;
				job_queue[i].current_wt = 0;
			}
		}
		systemtime++;
	}
	printf("\n\nALL PROCESSES ARE FINISHED.");
	for (i = 0; i < the_num_process; i++)
	{
		Priority_TA = Priority_TA + (job_queue[i].tatime) - (job_queue[i].arrivaltime) + 1;
		Priority_WT = Priority_WT + job_queue[i].wttime;
	}
	Priority_TA = Priority_TA / the_num_process;
	Priority_WT = Priority_WT / the_num_process;
	Priority_CPU_utilization = (systemtime - Priority_CPU_utilization) / systemtime * 100;
	return;
}

void RR()
{
	int i;
	int count = 0;
	initializing();
	RR_CPU_utilization = 0;
	while (count != the_num_process)
	{
		count = 0;
		arrive_readyqueue();
		printf("\n time= %d\n", systemtime);
		if (CPU_use != NULL)					// 1. there is a process using cpu -> the process cpu burst
		{
			CPU_use->current_CPU--;
			CPU_use->tq--;
			printf("	PROCESS   %d run", CPU_use->ID);
			if (CPU_use->current_CPU == 0)
			{
				cycle_update(CPU_use);
				if (CPU_use->current_cycle == 0)					// if cycle of the process is 0, finished
				{
					finish(CPU_use);
					CPU_use = NULL;
				}
				else				// if cycle of the process is not 0,
				{
					if (CPU_use->IOburst == 0)					// io burst x -> ready queue
						en_readyqueue(CPU_use);
					else					// io burst o -> waiting queue
						en_waitingqueue(CPU_use);
					CPU_use = NULL;
				}
			}
			else if (CPU_use->tq == 0 && empty_readyqueue() != 1)
			{
				CPU_use->tq = time_quantum;
				en_readyqueue(CPU_use);
				de_readyqueue();
			}
		}
		else if (empty_readyqueue() != 1)	// 2. there isnt a process using cpu, and there is process in ready queue
		{
			de_readyqueue();
			CPU_use->current_CPU--;
			CPU_use->tq--;
			printf("	PROCESS   %d run", CPU_use->ID);
			if (CPU_use->current_CPU == 0)
			{
				cycle_update(CPU_use);
				if (CPU_use->current_cycle == 0)					// if cycle of the process is 0, finished
				{
					finish(CPU_use);
					CPU_use = NULL;
				}
				else				// if cycle of the process is not 0,
				{
					if (CPU_use->IOburst == 0)					// io burst x -> ready queue
						en_readyqueue(CPU_use);
					else					// io burst o -> waiting queue
						en_waitingqueue(CPU_use);
					CPU_use = NULL;
				}
			}
			else if (CPU_use->tq == 0 && empty_readyqueue() != 1)
			{
				CPU_use->tq = time_quantum;
				en_readyqueue(CPU_use);
				de_readyqueue();
			}
		}
		else
		{
			printf("	IDLE");						// 3. ready queue is empty
			RR_CPU_utilization++;
		}

		IO_bursting();

		for (i = 0; i < the_num_process; i++)
		{
			if (job_queue[i].finish == 1)
				count++;
			else
				job_queue[i].tatime++;
			if (ready_queue.data[i] != NULL)
				ready_queue.data[i]->wttime++;
		}
		systemtime++;
	}
	printf("\n\nALL PROCESSES ARE FINISHED.");
	for (i = 0; i < the_num_process; i++)
	{
		RR_TA = RR_TA + (job_queue[i].tatime) - (job_queue[i].arrivaltime) + 1;
		RR_WT = RR_WT + job_queue[i].wttime;
	}
	RR_TA = RR_TA / the_num_process;
	RR_WT = RR_WT / the_num_process;
	RR_CPU_utilization = (systemtime - RR_CPU_utilization) / systemtime * 100;
	return;
}

void PRE_SJF()
{
	int i;
	int count = 0;
	process* temp;
	initializing();
	PRE_SJF_CPU_utilization = 0;
	while (count != the_num_process)
	{
		count = 0;
		arrive_readyqueue();
		if (empty_readyqueue != 0)
			shortest_job_sorting();					// sorting processes in ready queue by cpu burst
		printf("\n time= %d\n", systemtime);
		if (CPU_use != NULL)					// 1. there is a process using cpu 
		{
			if (ready_queue.data[ready_queue.out] != NULL)
			{
				if (ready_queue.data[ready_queue.out]->current_CPU < CPU_use->current_CPU)					// if there is a process in ready queue which has shorter cpu burst than the process which is using cpu
				{
					temp = ready_queue.data[ready_queue.out];					// swap.
					ready_queue.data[ready_queue.out] = CPU_use;
					CPU_use = temp;
				}
			}
			CPU_use->current_CPU--;
			printf("	PROCESS   %d run", CPU_use->ID);
			if (CPU_use->current_CPU == 0)
			{
				cycle_update(CPU_use);
				if (CPU_use->current_cycle == 0)					// if cycle of the process is 0, finished
				{
					finish(CPU_use);
					CPU_use = NULL;
				}
				else				// if cycle of the process is not 0,
				{
					if (CPU_use->IOburst == 0)					// io burst x -> ready queue
						en_readyqueue(CPU_use);
					else					// io burst o -> waiting queue
						en_waitingqueue(CPU_use);
					CPU_use = NULL;
				}
			}
		}
		else if (empty_readyqueue() != 1)	// 2. there isnt a process using cpu, and there is process in ready queue
		{
			de_readyqueue();
			CPU_use->current_CPU--;
			printf("	PROCESS   %d run", CPU_use->ID);
			if (CPU_use->current_CPU == 0)
			{
				cycle_update(CPU_use);
				if (CPU_use->current_cycle == 0)					// if cycle of the process is 0, finished
				{
					finish(CPU_use);
					CPU_use = NULL;
				}
				else				// if cycle of the process is not 0,
				{
					if (CPU_use->IOburst == 0)					// io burst x -> ready queue
						en_readyqueue(CPU_use);
					else					// io burst o -> waiting queue
						en_waitingqueue(CPU_use);
					CPU_use = NULL;
				}
			}
		}
		else
		{
			printf("	IDLE");						// 3. ready queue is empty
			PRE_SJF_CPU_utilization++;
		}

		IO_bursting();

		for (i = 0; i < the_num_process; i++)
		{
			if (job_queue[i].finish == 1)
				count++;
			else
				job_queue[i].tatime++;
			if (ready_queue.data[i] != NULL)
				ready_queue.data[i]->wttime++;
		}
		systemtime++;
	}
	printf("\n\nALL PROCESSES ARE FINISHED.");
	for (i = 0; i < the_num_process; i++)
	{
		PRE_SJF_TA = PRE_SJF_TA + (job_queue[i].tatime) - (job_queue[i].arrivaltime) + 1;
		PRE_SJF_WT = PRE_SJF_WT + job_queue[i].wttime;
	}
	PRE_SJF_TA = PRE_SJF_TA / the_num_process;
	PRE_SJF_WT = PRE_SJF_WT / the_num_process;
	PRE_SJF_CPU_utilization = (systemtime - PRE_SJF_CPU_utilization) / systemtime * 100;
	return;
}

void PRE_Priority()
{
	int i;
	int count = 0;
	process* temp;
	initializing();
	PRE_Priority_CPU_utilization = 0;
	while (count != the_num_process)
	{
		count = 0;
		arrive_readyqueue();
		if (empty_readyqueue != 0)
			priority_sorting();					// sorting processes by priority
		printf("\n time= %d\n", systemtime);
		if (CPU_use != NULL)					// 1. there is a process using cpu 
		{
			if (ready_queue.data[ready_queue.out] != NULL)
			{
				if (ready_queue.data[ready_queue.out]->priority < CPU_use->priority)					// if there is a process in ready queue which has shorter priority than the process which is using cpu
				{
					temp = ready_queue.data[ready_queue.out];					// swap.
					ready_queue.data[ready_queue.out] = CPU_use;
					CPU_use = temp;
				}
			}
			CPU_use->current_CPU--;
			printf("	PROCESS   %d run", CPU_use->ID);
			if (CPU_use->current_CPU == 0)
			{
				cycle_update(CPU_use);
				if (CPU_use->current_cycle == 0)					// if cycle of the process is 0, finished
				{
					finish(CPU_use);
					CPU_use = NULL;
				}
				else				// if cycle of the process is not 0,
				{
					if (CPU_use->IOburst == 0)					// io burst x -> ready queue
						en_readyqueue(CPU_use);
					else					// io burst o -> waiting queue
						en_waitingqueue(CPU_use);
					CPU_use = NULL;
				}
			}
		}
		else if (empty_readyqueue() != 1)	// 2. there isnt a process using cpu, and there is process in ready queue
		{
			de_readyqueue();
			CPU_use->current_CPU--;
			printf("	PROCESS   %d run", CPU_use->ID);
			if (CPU_use->current_CPU == 0)
			{
				cycle_update(CPU_use);
				if (CPU_use->current_cycle == 0)					// if cycle of the process is 0, finished
				{
					finish(CPU_use);
					CPU_use = NULL;
				}
				else				// if cycle of the process is not 0,
				{
					if (CPU_use->IOburst == 0)					// io burst x -> ready queue
						en_readyqueue(CPU_use);
					else					// io burst o -> waiting queue
						en_waitingqueue(CPU_use);
					CPU_use = NULL;
				}
			}
		}
		else
		{
			printf("	IDLE");						// 3. ready queue is empty
			PRE_Priority_CPU_utilization++;
		}

		IO_bursting();

		for (i = 0; i < the_num_process; i++)
		{
			if (job_queue[i].finish == 1)
				count++;
			else
				job_queue[i].tatime++;
			if (ready_queue.data[i] != NULL)
			{
				if (aging_or_not == 1)						// for aging, record waiting time of process
					ready_queue.data[i]->current_wt++;
				ready_queue.data[i]->wttime++;
			}
			if (job_queue[i].current_wt > 20)						// per wt of process>20, priority --
			{
				job_queue[i].priority--;
				job_queue[i].current_wt = 0;
			}
		}
		systemtime++;
	}
	printf("\n\nALL PROCESSES ARE FINISHED.");
	for (i = 0; i < the_num_process; i++)
	{
		PRE_Priority_TA = PRE_Priority_TA + (job_queue[i].tatime) - (job_queue[i].arrivaltime) + 1;
		PRE_Priority_WT = PRE_Priority_WT + job_queue[i].wttime;
	}
	PRE_Priority_TA = PRE_Priority_TA / the_num_process;
	PRE_Priority_WT = PRE_Priority_WT / the_num_process;
	PRE_Priority_CPU_utilization = (systemtime - PRE_Priority_CPU_utilization) / systemtime * 100;
	return;
}

void alg_analyze()
{
	printf("\n\n FCFS : average TA %f", FCFS_TA);
	printf("\n FCFS : average Waiting time %f", FCFS_WT);
	printf("\n FCFS : CPU_utilization %f", FCFS_CPU_utilization);
	printf("\n\n SJF : average TA %f", SJF_TA);
	printf("\n SJF : average Waiting time %f", SJF_WT);
	printf("\n SJF : CPU_utilization %f", SJF_CPU_utilization);
	printf("\n\n Priority : average Turnaround time %f", Priority_TA);
	printf("\n Priority : average Waiting time %f", Priority_WT);
	printf("\n Priority : CPU_utilization %f", Priority_CPU_utilization);
	printf("\n\n Round Robin : average Turnaround time %f", RR_TA);
	printf("\n Round Robin : average Waiting time %f", RR_WT);
	printf("\n Round Robin : CPU_utilization %f", RR_CPU_utilization);
	printf("\n\n Preemptive SJF : average TA %f", PRE_SJF_TA);
	printf("\n Preemptive SJF : average Waiting time %f", PRE_SJF_WT);
	printf("\n Preemptive SJF : CPU_utilization %f", PRE_SJF_CPU_utilization);
	printf("\n\n Preemptive Priority : average TA %f", PRE_Priority_TA);
	printf("\n Preemptive Priority : average Waiting time %f", PRE_Priority_WT);
	printf("\n Preemptive Priority : CPU_utilization %f", PRE_Priority_CPU_utilization);
}