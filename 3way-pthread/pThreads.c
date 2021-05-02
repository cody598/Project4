#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 4

#define ARRAY_SIZE 16000000
#define STRING_SIZE 16
#define ALPHABET_SIZE 26

pthread_mutex_t mutexsum;			// mutex for char_counts

char line_array[ARRAY_SIZE][STRING_SIZE];
int line_avg[ALPHABET_SIZE];			// count of individual characters


void init_arrays()
{
  int i, j, err;
  
  pthread_mutex_init(&mutexsum, NULL);
  FILE *fd;

   fd = fopen( "/homes/dan/625/wiki_dump.txt", "r" );
   for ( i = 0; i < ARRAY_SIZE; i++ )  {
      err = fscanf( fd, "%[^\n]\n", line_array[i]);
      if( err == EOF ) break;
   }

  for ( i = 0; i < ARRAY_SIZE; i++ ) {
  	line_avg[i] = 0.0;
  }
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
	int i, j, charLoc;
	float local_line_avg[ARRAY_SIZE];
	int startPos, endPos;

	#pragma omp private(myID,theChar,charLoc,local_char_count,startPos,endPos,i,j)
	{
		startPos = myID * (ARRAY_SIZE / NUM_THREADS);
		endPos = startPos + (ARRAY_SIZE / NUM_THREADS);

		printf("myID = %d startPos = %d endPos = %d \n", myID, startPos, endPos);

						// init local count array
		for ( i = 0; i < ARRAY_SIZE; i++ ) {
			local_line_avg[i] = 0.0;
		}
		
		// count up our section of the global array
		for ( i = startPos; i < endPos; i++) {
			local_line_avg[i]=find_avg(line_array[i], strlen(line_array[i]));
		}

		pthread_mutex_lock (&mutexsum);
		// sum up the partial counts into the global arrays
		
		#pragma omp critical
		{
			for ( i = 0; i < ARRAY_SIZE; i++ ) {
		 		line_avg[i] += local_line_avg[i];
			}
		}
		pthread_mutex_unlock (&mutexsum);
		pthread_exit(NULL);
	}
}

void print_results(float the_line_avg[])
{
  int i,j, total = 0;

  					// then print out the totals
  for ( i = 0; i < ARRAY_SIZE; i++ ) {
	printf("%d: %.1f\n", i, the_line_avg[i]);
  }
}

main() {

	int i, rc;
	pthread_t threads[NUM_THREADS];
	pthread_attr_t attr;
	void *status;

	/* Initialize and set thread detached attribute */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	init_arrays();

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

	print_results(line_avg);

	pthread_mutex_destroy(&mutexsum);
	printf("Main: program completed. Exiting.\n");
	pthread_exit(NULL);
}

