/* Finds the average values of read character lines from file.	
   openMP - Parallel
   Project 4 - Team 20 
*/

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include "sys/types.h"
#include "sys/sysinfo.h"

#define ARRAY_SIZE 100000
#define STRING_SIZE 2001


int NUM_THREADS = 4;
float line_avg[ARRAY_SIZE];
char lines[ARRAY_SIZE][STRING_SIZE];
FILE *fd;

typedef struct{
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


void readFile()
{
	int err,i;
	fd = fopen( "/homes/dan/625/wiki_dump.txt", "r" );
	for ( i = 0; i < ARRAY_SIZE; i++ )  {
      err = fscanf( fd, "%[^\n]\n", lines[i]);
      if( err == EOF ) break;
	}
	fclose( fd );
}

//Finds the average of the characters
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


//Removed file pointer argument in this version as it seems unnecessary/unused.
void *count_array(int myID)
{
    int i, startPos, endPos;
    float local_line_avg[ARRAY_SIZE];
    //Directive that instructs each thread to keep its own copy of function's private variables.
    #pragma omp private(myID, i, startPos, endPos, local_line_avg)
    {
        startPos = ((int) myID) * (ARRAY_SIZE / NUM_THREADS);
        endPos = startPos + (ARRAY_SIZE / NUM_THREADS);
        printf("myID = %d startPos = %d endPos = %d \n", (int) myID, startPos, endPos);
        
        //initialize local line average
        for ( i = 0; i < ARRAY_SIZE; i++ ) {
			local_line_avg[i] = 0.0;
		}

        for ( i = startPos; i < endPos; i++) {
            local_line_avg[i] = find_avg(lines[i], strlen(lines[i]));
        }

        //Critical section is indicated here, should wrap the addition of all local character counts to the global.
        #pragma omp critical
        {
            for ( i = startPos; i < endPos; i++) {
                line_avg[i] += local_line_avg[i];
            }
        }
    }
}

void print_results(float lineavg[])
{
    int i;
    for(i = 0; i<ARRAY_SIZE; i++)
	{
		printf("%d: %.1f\n", i, lineavg[i]);
    }
}

main()
{
	// Sets the number of threads.
	NUM_THREADS = atoi(getenv("SLURM_NTASKS"));		
    omp_set_num_threads(NUM_THREADS);
    struct timeval t1, t2, t3, t4;
    double timeElapsedInit, timeElapsedProcess, timeElapsedPrint, timeElapsedTotal;
    processMem_t memory;
	
    /* Timing analysis begins */
    gettimeofday(&t1, NULL);
    
    readFile();
    
    gettimeofday(&t2,NULL); //t2 - t1 

    #pragma omp parallel
    {
        count_array(omp_get_thread_num());
    }
	GetProcessMemory(&memory);
    gettimeofday(&t3, NULL); //t3 - t2 
    
    print_results(line_avg);
    
    gettimeofday(&t4, NULL); //t4 - t1 
	
    //total program time
    timeElapsedTotal = (t4.tv_sec - t1.tv_sec) * 1000.0; //Convert to to milliseconds
    timeElapsedTotal += (t4.tv_usec - t1.tv_usec) / 1000.0;
        
    printf("Tasks: %s\nTotal Time: %fms\n", getenv("SLURM_NTASKS"), timeElapsedTotal);
	printf("DATA, %s,%fms\n", getenv("SLURM_NTASKS"), timeElapsedTotal);
	printf("size = %d, Node: %s, vMem %u KB, pMem %u KB\n", NUM_THREADS, getenv("HOSTNAME"), memory.virtualMem, memory.physicalMem);

	printf("Main: program completed. Exiting.\n");

}

