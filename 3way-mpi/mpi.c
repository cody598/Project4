/* Finds the average values of read character lines from file.	
   MPI - Parralel
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

#define MAX_LINE_LENGTH 2200
#define MAX_LINES 1000000


/* Global variables. */
int NUM_THREADS = 4;
float line_average[MAX_LINES];
float local_average[MAX_LINES];

/* Timekeeping variables. */
struct timeval t1, t2;
double overalltime;

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
		//printf("%s", line);
		if (strncmp(line, "VmSize:", 7) == 0) {
			processMem->virtualMem = parseLine(line);
		}

		if (strncmp(line, "VmRSS:", 6) == 0) {
			processMem->physicalMem = parseLine(line);
		}
	}
	fclose(file);
}

/* Find average char value of a passed in line. */
float find_avg(char* line, int nchars) {
   int i, j;
   float sum = 0;

   for ( i = 0; i < nchars; i++ ) {
      sum += ((int) line[i]);
   }

   if (nchars > 0) 
	return sum / (float) nchars;
   else
	return 0.0;
}

/* Based on rank the file is partitioned. Once partitioned, averages for each line will be found. */
void read_line_avg(int rank, FILE * fd)
{
	char tempBuffer[MAX_LINE_LENGTH];
	int fileSize;
	int	chunks;
	int beginning;
	int currentLine = 0;

	/* Get file size and set the chunks from the total file size*/
	fseek(fd, 0, SEEK_END);
	fileSize = ftell(fd);
	chunks = fileSize/NUM_THREADS;

	/* Based on rank, set the beginning position. */
	if(rank == 0)
		fseek(fd, 0, SEEK_SET);

	/* Discard line the partition is not placed correctly. */
	if(rank > 0)
	{
		fseek(fd, rank*chunks-1, SEEK_SET);
		fscanf(fd, "%[^\n]\n", tempBuffer);
		if(strcmp(tempBuffer,"") == 0)
		{
			fseek(fd, rank*chunks, SEEK_SET);
		}
	}

	/* Find current line number. */
	beginning = ftell(fd);
	fseek(fd, 0, SEEK_SET);
	while(ftell(fd) < beginning)
	{
		fscanf(fd, "%[^\n]\n", tempBuffer);
		currentLine++;
	}

	/* Sets file start position */
	fseek(fd, beginning, SEEK_SET);
		
	/* End of File not reached and new block has yet to be assigned. */
	while(ftell(fd) <= (rank+1)*chunks
		&& fscanf(fd, "%[^\n]\n", tempBuffer) != EOF)
	{
		/* Newline char verification.  */
		if(strcmp(tempBuffer,"") == 0)
		{
			fseek(fd, (rank*chunks)+1, SEEK_SET);
		}

		/* Find average of the temporary buffer. */
		else
		{
			int lineLength = strlen(tempBuffer);

			if(currentLine > MAX_LINES)
			{				
				printf("Max lines reached!\n");
				break;
			}

			/* Save line average into local array. */
			local_average[currentLine] = find_avg(tempBuffer, lineLength);
			currentLine++;
		}
	}
}

/* Prints the results. */
void PrintLineAverages()
{
	int i;
	for(i = 0; i < MAX_LINES; i++)
	{
		/* Print mean. */
		printf("%d: %.1f\n", i, line_average[i]);
	}
}

main(int argc, char *argv[])
{
	int i, rc;
	int rank, numtasks;
	FILE * fd;
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

    /* Start performance metrics. */
    if(rank == 0)
    {
		gettimeofday(&t1, NULL);
		printf("DEBUG: starting time on %s\n", getenv("HOSTNAME"));
    }
 
	NUM_THREADS = numtasks;

	/* Important Data Retreival and Setup. */	
	GetProcessMemory(&myMem);
	printf("size = %d rank = %d, Node: %s, vMem %u KB, pMem %u KB\n", numtasks, rank, getenv("HOSTNAME"), myMem.virtualMem, myMem.physicalMem);

	/* Get file. */
	fd = fopen("/homes/dan/625/wiki_dump.txt","r");
	if(fd == NULL)
	{
		printf("file not found\n");
		exit(-1);
	}
	
	/* Gets the line averages of the read-in-file */
	read_line_avg(rank, fd);
	fclose(fd);

	/* Merge local arrays into the global array. */
	MPI_Reduce(local_average, line_average, MAX_LINES, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

	/* Print once complete. */
	if(rank == 0)
	{
		PrintLineAverages();
		gettimeofday(&t2, NULL);
		overalltime = (t2.tv_sec - t1.tv_sec) * 1000.0; //convert to milliseconds
		overalltime += (t2.tv_usec - t1.tv_usec) / 1000.0; // convert to milliseconds
		printf("Tasks: %s, Elapsed Time: %fms\n",  getenv("SLURM_NTASKS"),  overalltime);
	}

	MPI_Finalize();
	return 0;
}