#ifndef _REENTRANT 
#define _REENTRANT 
#endif 
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <limits.h>

#define MAXSIZE 10000  
#define MAXWORKERS 10   

pthread_mutex_t resultsMutex; 
pthread_mutex_t rowMutex;     
int numWorkers;                
int nextRow = 0;               

int globalSum = 0;
int globalMaxVal;
int globalMaxRow;
int globalMaxCol;
int globalMinVal;
int globalMinRow;
int globalMinCol;

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
int matrix[MAXSIZE][MAXSIZE]; /* matrix */

void *Worker(void *);


int main(int argc, char *argv[]){
    int i, j;
    long l;
    pthread_attr_t attr;
    pthread_t workerId[MAXWORKERS];

    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    pthread_mutex_init(&resultsMutex, NULL);
    pthread_mutex_init(&rowMutex, NULL);

     /* read command line args if any */
    size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
    numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
    if (size > MAXSIZE) size = MAXSIZE;
    if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;
    stripSize = size/numWorkers;

    for (i = 0; i < size; i++) {
	  for (j = 0; j < size; j++) {
          matrix[i][j] = rand()%99;
	  }
    }

    globalMaxVal = matrix[0][0];
    globalMaxRow = 0;
    globalMaxCol = 0;
    globalMinVal = matrix[0][0];
    globalMinRow = 0;
    globalMinCol = 0;

    #ifdef DEBUG
  for (i = 0; i < size; i++) {
	  printf("[ ");
	  for (j = 0; j < size; j++) {
	    printf(" %d", matrix[i][j]);
	  }
	  printf(" ]\n");
  }
    #endif

    start_time = read_timer();
    for (l = 0; l < numWorkers; l++)
        pthread_create(&workerId[l], &attr, Worker, (void *) l);

    /* wait for all workers to complete */
    for (l = 0; l < numWorkers; l++)
        pthread_join(workerId[l], NULL);

    /* get end time */
    end_time = read_timer();

    printf("The total is %d\n", globalSum);
    printf("The highest value is %d at row %d and column %d\n", globalMaxVal, globalMaxRow, globalMaxCol);
    printf("The lowest value is %d at row %d and column %d\n", globalMinVal, globalMinRow, globalMinCol);
    printf("The execution time is %g sec\n", end_time - start_time);

    pthread_mutex_destroy(&resultsMutex);
    pthread_mutex_destroy(&rowMutex);
    pthread_attr_destroy(&attr);

    return 0;    
}

void *Worker(void *arg) {
  long myid = (long) arg;
  int localSum, j, myRow;
  int localMaxVal, localMaxRow, localMaxCol;
  int localMinVal, localMinRow, localMinCol;
  bool firstElement = true;  

#ifdef DEBUG
  printf("worker %d (pthread id %d) has started\n", myid, pthread_self());
#endif

  localSum = 0;
  
  while (true) {
    pthread_mutex_lock(&rowMutex);
    
    if (nextRow >= size) {
      pthread_mutex_unlock(&rowMutex);
      break;
    }
    
    myRow = nextRow;
    nextRow++;
    
    pthread_mutex_unlock(&rowMutex);
    
    for (j = 0; j < size; j++) {
      localSum += matrix[myRow][j];
      
      if (firstElement) {
        localMaxVal = matrix[myRow][j];
        localMaxRow = myRow;
        localMaxCol = j;
        localMinVal = matrix[myRow][j];
        localMinRow = myRow;
        localMinCol = j;
        firstElement = false;
      }
      
      if (matrix[myRow][j] > localMaxVal) {
        localMaxVal = matrix[myRow][j];
        localMaxRow = myRow;
        localMaxCol = j;
      }
      
      if (matrix[myRow][j] < localMinVal) {
        localMinVal = matrix[myRow][j];
        localMinRow = myRow;
        localMinCol = j;
      }
    }
    
#ifdef DEBUG
    printf("worker %d processed row %d\n", myid, myRow);
#endif
  }

  pthread_mutex_lock(&resultsMutex);
  
  globalSum += localSum;
  
  if (localMaxVal > globalMaxVal) {
    globalMaxVal = localMaxVal;
    globalMaxRow = localMaxRow;
    globalMaxCol = localMaxCol;
  }
  
  if (localMinVal < globalMinVal) {
    globalMinVal = localMinVal;
    globalMinRow = localMinRow;
    globalMinCol = localMinCol;
  }
  
  pthread_mutex_unlock(&resultsMutex);

#ifdef DEBUG
  printf("worker %d has finished\n", myid);
#endif

  return NULL;
}