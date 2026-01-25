/* matrix summation using pthreads

   features: uses a barrier; the Worker[0] computes
             the total sum from partial sums computed by Workers
             and prints the total sum to the standard output

   usage under Linux:
     gcc matrixSum.c -lpthread
     a.out size numWorkers

*/
#ifndef _REENTRANT 
#define _REENTRANT 
#endif 
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#define MAXSIZE 10000  /* maximum matrix size */
#define MAXWORKERS 10   /* maximum number of workers */

pthread_mutex_t barrier;  /* mutex lock for the barrier */
pthread_cond_t go;        /* condition variable for leaving */
int numWorkers;           /* number of workers */ 
int numArrived = 0;       /* number who have arrived */

/* a reusable counter barrier */
void Barrier() {
  pthread_mutex_lock(&barrier); // Locks the mutex so that only one thread can execute at a time
  numArrived++; // Increment count when the one more thread comes to the barrier. 
  if (numArrived == numWorkers) {
    numArrived = 0;
    pthread_cond_broadcast(&go); // When all workers have arrived, we reset the counter and release all threads
  } else
    pthread_cond_wait(&go, &barrier); // Wait for all workers
  pthread_mutex_unlock(&barrier); // Unlock mutex so all threads can progress. 
}

/* timer */
double read_timer() {
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;
    if( !initialized )
    {
        gettimeofday( &start, NULL );
        initialized = true;
    }
    gettimeofday( &end, NULL );
    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

double start_time, end_time; /* start and end times */
int size, stripSize;  /* assume size is multiple of numWorkers */
int sums[MAXWORKERS]; /* partial sums */
int matrix[MAXSIZE][MAXSIZE]; /* matrix */

typedef struct Result
{
  int value;
  int row;
  int col;
} Result;

Result maxResults[MAXWORKERS];
Result minResults[MAXWORKERS];


void *Worker(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  int i, j;
  long l; /* use long in case of a 64-bit system */
  pthread_attr_t attr;
  pthread_t workerid[MAXWORKERS];

  /* set global thread attributes */
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  /* initialize mutex and condition variable */
  pthread_mutex_init(&barrier, NULL);
  pthread_cond_init(&go, NULL);

  /* read command line args if any */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;
  stripSize = size/numWorkers;

  /* initialize the matrix */
  for (i = 0; i < size; i++) {
	  for (j = 0; j < size; j++) {
          matrix[i][j] = rand()%99;
	  }
  }

  /* print the matrix */
#ifdef DEBUG
  for (i = 0; i < size; i++) {
	  printf("[ ");
	  for (j = 0; j < size; j++) {
	    printf(" %d", matrix[i][j]);
	  }
	  printf(" ]\n");
  }
#endif

  /* do the parallel work: create the workers */
  start_time = read_timer();
  for (l = 0; l < numWorkers; l++)
    pthread_create(&workerid[l], &attr, Worker, (void *) l);
  pthread_exit(NULL);
}

/* Each worker sums the values in one strip of the matrix.
   After a barrier, worker(0) computes and prints the total */
void *Worker(void *arg) {
  long myid = (long) arg;
  int total, i, j, first, last;

#ifdef DEBUG
  printf("worker %d (pthread id %d) has started\n", myid, pthread_self());
#endif

  /* determine first and last rows of my strip */
  first = myid*stripSize;
  last = (myid == numWorkers - 1) ? (size - 1) : (first + stripSize - 1);

  /* sum values in my strip */
  total = 0;
  for (i = first; i <= last; i++)
    for (j = 0; j < size; j++)
      total += matrix[i][j];
  sums[myid] = total;

  // Find Max Values in strip

  int maxVal = matrix[first][0];
  int maxRow = first;
  int maxCol = 0;
  for (int i = first; i <=last; i++)
  {
    for (int j = 0; j < size; j++)
    {
        if (matrix[i][j] > maxVal)
        {
          maxVal = matrix[i][j];
          maxRow = i;
          maxCol = j;
        }
    }
  }
  maxResults[myid].value = maxVal;
  maxResults[myid].col = maxCol;
  maxResults[myid].row = maxRow;

   // Find Min Values in strip

  int minVal = matrix[first][0];
  int minRow = first;
  int minCol = 0;
  for (int i = first; i <=last; i++)
  {
    for (int j = 0; j < size; j++)
    {
        if (matrix[i][j] < minVal)
        {
          minVal = matrix[i][j];
          minRow = i;
          minCol = j;
        }
    }
  }

  minResults[myid].value = minVal;
  minResults[myid].col = minCol;
  minResults[myid].row = minRow;
  

  Barrier();
  if (myid == 0) {
    total = 0;
    for (i = 0; i < numWorkers; i++){
      total += sums[i];
    }

    int maxValue = maxResults[0].value;
    int maxRow = maxResults[0].row;
    int maxCol = maxResults[0].col;
    for (int i = 0; i < numWorkers; i++)
    {
      if (maxResults[i].value > maxValue)
      {
        maxValue = maxResults[i].value;
        maxRow = maxResults[i].row;
        maxCol = maxResults[i].col;
      }
    }

    int minValue = minResults[0].value;
    int minRow = minResults[0].row;
    int minCol = minResults[0].col;
    for (int i = 0; i < numWorkers; i++)
    {
      if (minResults[i].value < minValue)
      {
        minValue = minResults[i].value;
        minRow = minResults[i].row;
        minCol = minResults[i].col;
      }
    }

    /* get end time */
    end_time = read_timer();
    /* print results */
    printf("The total is %d\n", total);
    printf("The highest value is %d at row %d and column %d\n", maxValue, maxRow, maxCol);
    printf("The lowest value is %d at row %d and column %d\n", minValue, minRow, minCol);
    printf("The execution time is %g sec\n", end_time - start_time);
  }
}