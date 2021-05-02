/* Finds the average values of read character lines from file.	
   openMP - Parralel
   Project 4 - Team 20 
*/


#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>
#include "sys/types.h"
#include "sys/sysinfo.h"

#define ARRAY_SIZE 1000000
#define NUM_THREADS 16
#define STRING_SIZE 2001

#define MAX_CORES 32
#define PRINTABLE_CHAR_MIN 32
#define PRINTABLE_CHAR_MAX 126

float line_avg[ARRAY_SIZE];

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
    char theChar;
    int i, startPos, endPos, err;
    float local_line_avg[ARRAY_SIZE];
	int currentLine = 0;
    FILE *fd;
    char line[STRING_SIZE];
	int nchars;
	int nlines = 0;
	
    //Directive that instructs each thread to keep its own copy of function's private variables.
    #pragma omp private(myID, theChar, i, startPos, endPos,local_line_avg,nchars,err,nlines, line, err)
    {
        startPos = ((int) myID) * (ARRAY_SIZE / NUM_THREADS);
        endPos = startPos + (ARRAY_SIZE / NUM_THREADS);
        printf("myID = %d startPos = %d endPos = %d \n", (int) myID, startPos, endPos);
        
        //initialize local line average
        for ( i = startPos; i < endPos; i++ ) {
			local_line_avg[i] = 0.0;
		}

		fd = fopen( "/homes/dan/625/wiki_dump.txt", "r" );

	    while(nlines < startPos)
		{
			err = fscanf( fd, "%[^\n]\n", line);
			if( err == EOF ) break;
			nlines++;
		}
		for(int i = startPos; i < endPos; i++)
		{
			err = fscanf( fd, "%[^\n]\n", line);
			if( err == EOF ) break;
			nchars = strlen( line );
			local_line_avg[i] = find_avg(line, nchars);
		} 
		
		fclose( fd );
		
        //Critical section is indicated here, should wrap the addition of all local character counts to the global.
        #pragma omp critical
        {
            for ( i = startPos; i < endPos; i++) {
                line_avg[i] += local_line_avg[i];
            }
        }
    }
}

void print_results(float line_avg[]){
    int i;
    for(i = 0; i<ARRAY_SIZE; i++){
             printf("%d: %.1f\n", i, line_avg[i]);

    }
}

main()
{
    // Sets the number of threads.
    omp_set_num_threads(NUM_THREADS);
    struct timeval t1, t2, t3, t4;
    double timeElapsedInit, timeElapsedProcess, timeElapsedPrint, timeElapsedTotal;
    processMem_t memory;
    
    
    /* Timing analysis begins */
    gettimeofday(&t1, NULL);
    
    #pragma omp parallel
    {
        count_array(omp_get_thread_num());
    }
	
    gettimeofday(&t2, NULL); //t3 - t2 for processing
    
    print_results(line_avg);
    
    gettimeofday(&t3, NULL); //t4 - t1 for whole program
    
    //init_array time
    timeElapsedInit = (t2.tv_sec - t1.tv_sec) * 1000.0; //Time in seconds converted to milliseconds
    timeElapsedInit += (t2.tv_usec - t1.tv_usec) / 1000.0;

    // Data Process Time (to find avg)
    timeElapsedProcess = (t2.tv_sec - t1.tv_sec) * 1000.0; //Time in seconds converted to milliseconds
    timeElapsedProcess += (t2.tv_usec - t1.tv_usec) / 1000.0;

    // Data Printing Time
    timeElapsedPrint = (t3.tv_sec - t2.tv_sec) * 1000.0; //Time in seconds converted to milliseconds
    timeElapsedPrint += (t3.tv_usec - t2.tv_usec) / 1000.0;

    //total program time
    timeElapsedTotal = (t3.tv_sec - t1.tv_sec) * 1000.0; //Time in seconds converted to milliseconds
    timeElapsedTotal += (t3.tv_usec - t1.tv_usec) / 1000.0;
        
    printf("Tasks: %s\nProcess Elapsed Time: %fms\nPrint Elapsed Time: %fms\nTotal Elapsed Time: %fms\n", getenv("SLURM_NTASKS"), timeElapsedProcess, timeElapsedPrint, timeElapsedTotal);
    
    GetProcessMemory(&memory);
	printf("size = %d, Node: %s, vMem %u KB, pMem %u KB\n", NUM_THREADS, getenv("HOSTNAME"), memory.virtualMem, memory.physicalMem);

	printf("Main: program completed. Exiting.\n");

}

