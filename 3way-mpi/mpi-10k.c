/* Finds the average values of read character lines from file.	
   MPI - Parallel
   Project 4 - Team 20 
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <mpi.h>
#include <stdint.h>
#include "sys/types.h"
#include "sys/sysinfo.h"
#include <math.h>

#define MAXIMUM_TASKS 32
#define STRING_SIZE 2001
#define ARRAY_SIZE 10000

/* Global variables. */
float NUM_THREADS;
unsigned int thread_locations[MAXIMUM_TASKS];
float line_averages[ARRAY_SIZE];
float local_average[ARRAY_SIZE];

typedef struct {
	uint32_t virtualMem;
	uint32_t physicalMem;
} processMem_t;

int parseLine(char *line) {
	// This assumes that a digit will be found and the line ends in " Kb".
	int i = strlen(line);
	const char *p = line;
	while (*p < '0' || *p > '9') p++;
	line[i - 3] = '\0';
	i = atoi(p);
	return i;
}

void GetProcessMemory(processMem_t* processMem) {
	FILE *file = fopen("/proc/self/status", "r");
	char line[128];

	while (fgets(line, 128, file) != NULL) {
		if (strncmp(line, "VmSize:", 7) == 0) {
			processMem->virtualMem = parseLine(line);
		}

		if (strncmp(line, "VmRSS:", 6) == 0) {
			processMem->physicalMem = parseLine(line);
		}
	}
	fclose(file);
}

/* Find average char value of a line. */
float find_line_average(char* line, int nchars) 
{
   int i, j;
   float sum = 0;

   for ( i = 0; i < nchars; i++ ) 
   {
      sum += ((int) line[i]);
   }

   if (nchars > 0) 
	return sum / (float) nchars;
   else
	return 0.0;
}

/* Computes the local average array. Work-load distributed equally.*/
void find_avg(int rank, FILE * fp)
{
	char tempBuffer[STRING_SIZE];
	int currentLine = rank * (ARRAY_SIZE/NUM_THREADS);

	fseek(fp, thread_locations[rank], SEEK_SET);
	
	/* While not at EOF and not beyond assigned section ... */
	while(currentLine < (rank+1) * (ARRAY_SIZE/NUM_THREADS)
		&& fscanf(fp, "%[^\n]\n", tempBuffer) != EOF)
	{
			/* Find and save average of line of char locally */
			int lineLength = strlen(tempBuffer);
			local_average[currentLine] = find_line_average(tempBuffer, lineLength);
			currentLine++;
	}
}


/* Prints the char averages. */
void printResults()
{
	int i;
	for(i = 0; i<ARRAY_SIZE; i++)
	{
		printf("%d: %.1f\n", i, line_averages[i]);
	}
}

main(int argc, char *argv[])
{
	/* Time variables. */
	struct timeval t1, t2;
	double timeElapsedTotal;
	
	int i, rc;
	int rank, numtasks;
	FILE * fp;
	MPI_Status Status;
	processMem_t myMem; 

	/* MPI Setup. */
	rc = MPI_Init(&argc,&argv);
	if (rc != MPI_SUCCESS) 
	{
		printf ("Error starting MPI program. Terminating.\n");
	    MPI_Abort(MPI_COMM_WORLD, rc);
    }
    MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    /* Start performance */
    if(rank == 0)
    {
		gettimeofday(&t1, NULL);
		printf("DEBUG: starting time on %s\n", getenv("HOSTNAME"));
    }
 
	NUM_THREADS = numtasks;

	fp = fopen("/homes/dan/625/wiki_dump.txt","r");
	if(fp == NULL)
	{
		printf("file not found\n");
		exit(-1);
	}

	/* Distributes workload */
	if(rank == 0)
	{
		char tempBuffer[STRING_SIZE];
		i = 1;
		int currentLine = 0;

		/* Set starting position. */
		fseek(fp, 0, SEEK_SET);
		thread_locations[0] = ftell(fp);
		int prevPos = ftell(fp);

		/* Count lines and save their positions in file in the array. */
		while(currentLine < ARRAY_SIZE && fscanf(fp, "%[^\n]\n", tempBuffer) != EOF)
		{			
			if(currentLine == i * ARRAY_SIZE/NUM_THREADS)
			{
				thread_locations[i] = prevPos;
				i++;
			}
			prevPos = ftell(fp);
			currentLine++;
		}

		fseek(fp, 0, SEEK_SET);	
	}
	MPI_Bcast(thread_locations, NUM_THREADS+1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

	find_avg(rank, fp);
	
	fclose(fp);

	/* Merge local to global arrays */
	MPI_Reduce(local_average, line_averages, ARRAY_SIZE, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

	/* Print results and record important data */
	if(rank == 0)
	{
		printResults();
		gettimeofday(&t2, NULL);
		timeElapsedTotal = (t2.tv_sec - t1.tv_sec) * 1000.0; // Convert to ms
		timeElapsedTotal += (t2.tv_usec - t1.tv_usec) / 1000.0; // Convert to ms
			/* Performance metrics. */	
		GetProcessMemory(&myMem);
		printf("size = %d rank = %d, Node: %s, vMem %u KB, pMem %u KB\n", numtasks, rank, getenv("HOSTNAME"), myMem.virtualMem, myMem.physicalMem);
		printf("Tasks: %s, Elapsed Time: %fms\n",  getenv("SLURM_NTASKS"),  timeElapsedTotal);
		printf("DATA, %s,%fms\n", getenv("SLURM_NTASKS"), timeElapsedTotal);
	}

	MPI_Finalize();
	return 0;
}