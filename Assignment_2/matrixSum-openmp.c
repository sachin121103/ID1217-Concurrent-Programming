/* matrix summation using OpenMP

   usage with gcc (version 4.2 or higher required):
     gcc -O -fopenmp -o matrixSum-openmp matrixSum-openmp.c 
     ./matrixSum-openmp size numWorkers

*/

#include <omp.h>
#include <stdio.h>
#include <time.h>


double start_time, end_time;

#define MAXSIZE 10000  /* maximum matrix size */
#define MAXWORKERS 8   /* maximum number of workers */

int numWorkers;
int size; 
int matrix[MAXSIZE][MAXSIZE];
void *Worker(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  int i, j, total=0;
  int max_val = -1;
  int min_val = 101;
  int max_row, max_col, min_row, min_col;

  /* read command line args if any */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;

  omp_set_num_threads(numWorkers);

  srand(time(NULL));
  /* initialize the matrix */
  for (i = 0; i < size; i++) {
    //  printf("[ ");
	  for (j = 0; j < size; j++) {
      matrix[i][j] = rand()%99;
      //	  printf(" %d", matrix[i][j]);
	  }
	  //	  printf(" ]\n");
  }

  start_time = omp_get_wtime();
#pragma omp parallel for reduction (+:total) private(j) // This is the part of the code that keeps a running total, then sums everything up
  for (i = 0; i < size; i++)
    for (j = 0; j < size; j++){
      total += matrix[i][j];
    }

#pragma omp parallel
{
  int local_max = -1;
  int local_min = 101;
  int local_max_row, local_max_col, local_min_row, local_min_col;

  #pragma omp for
  for (int i = 0; i < size; i++)
  {
    for (int j = 0; j < size; j++)
    {
      if (matrix[i][j] > local_max)
      {
        local_max = matrix[i][j];
        local_max_row = i;
        local_max_col = j;
      }

      if (matrix[i][j] < local_min)
      {
        local_min = matrix[i][j];
        local_min_row = i;
        local_min_col = j;
      }
    }
  }

  #pragma omp critical
  {
    if (local_max > max_val)
    {
      max_val = local_max;
      max_col = local_max_col;
      max_row = local_max_row;
    }

    if (local_min < min_val)
    {
      min_val = local_min;
      min_col = local_min_col;
      min_row = local_min_row;
    }
  }
}

  end_time = omp_get_wtime();

  printf("the total is %d\n", total);
  printf("The highest value is %d at [%d, %d]\n", max_val, max_row, max_col);
  printf("The lowest value is %d at [%d, %d]\n", min_val, min_row, min_col);
  printf("it took %g seconds\n", end_time - start_time);

}


// Notable way to improve efficiency: Combine the directives for sum, min and max and compute them together. 