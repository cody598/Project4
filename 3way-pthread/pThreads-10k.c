/* Finds the average values of read character lines from file.	
   pThreads - Parallel
   Project 4 - Team 20 
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>
#include "sys/types.h"
#include "sys/sysinfo.h"


#define ARRAY_SIZE 10000
#define STRING_SIZE 2001
#define ALPHABET_SIZE 26

pthread_mutex_t mutexsum;			// mutex for char_counts
int NUM_THREADS = 2;
float line_avg[ARRAY_SIZE];			// count of individual characters
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

void *count_array(int myID)
{
    char theChar;
    int i, startPos, endPos, err;
	int currentLine = 0;
    char line[STRING_SIZE];
	int nchars;
	int nlines = 0;

	startPos = myID * (ARRAY_SIZE / NUM_THREADS);
	endPos = startPos + (ARRAY_SIZE / NUM_THREADS);

	printf("myID = %d startPos = %d endPos = %d \n", myID, startPos, endPos);
	
	for(i = startPos; i < endPos; i++)
	{
		nchars = strlen( lines[i] );
		line_avg[i] = find_avg(lines[i], nchars);
	} 

	pthread_exit(NULL);
}

void print_results(float lineavg[])
{
  int i,j, total = 0;

  // then print out the totals
  for ( i = 0; i < ARRAY_SIZE; i++ ) {
	printf("%d: %.1f\n", i, lineavg[i]);
  }
}

main() {

	int i, rc;
	NUM_THREADS = atoi(getenv("SLURM_NTASKS"));
	pthread_t threads[NUM_THREADS];
	pthread_attr_t attr;
	void *status;
	struct timeval t1, t2, t3;
    double timeElapsedTotal;
    processMem_t memory;
	
	pthread_mutex_init(&mutexsum, NULL);
	
	readFile();

	/* Timing analysis begins */
    gettimeofday(&t1, NULL);
	printf("DEBUG: starting time on %s\n", getenv("HOSTNAME"));
	
	/* Initialize and set thread detached attribute */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	for (i = 0; i < NUM_THREADS; i++ ) {
	      rc = pthread_create(&threads[i], &attr, count_array, (void *)i);
	      if (rc) {
	        printf("ERROR; return code from pthread_create() is %d\n", rc);
		exit(-1);
	      }
	}
	
	/* Free attribute and wait for the other threads */
	pthread_attr_destroy(&attr);
	for(i=0; i<NUM_THREADS; i++) {
	     rc = pthread_join(threads[i], &status);
	     if (rc) {
		   printf("ERROR; return code from pthread_join() is %d\n", rc);
		   exit(-1);
	     }
	}
	
	gettimeofday(&t2, NULL); 

	print_results(line_avg);

	gettimeofday(&t3, NULL); 


    //total program time
    timeElapsedTotal = (t3.tv_sec - t1.tv_sec) * 1000.0; //Converted to milliseconds
    timeElapsedTotal += (t3.tv_usec - t1.tv_usec) / 1000.0;

	pthread_mutex_destroy(&mutexsum);
	
	printf("Tasks: %s\n Total Elapsed Time: %fms\n", getenv("SLURM_NTASKS"), timeElapsedTotal);
	printf("DATA, %s,%fms\n", getenv("SLURM_NTASKS"), timeElapsedTotal);
    GetProcessMemory(&memory);
	
	printf("size = %d, Node: %s, vMem %u KB, pMem %u KB\n", NUM_THREADS, getenv("HOSTNAME"), memory.virtualMem, memory.physicalMem);
	printf("Main: program completed. Exiting.\n");
	
	pthread_exit(NULL);
}

