#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>


void swap (int* a, int* b){

    int temp = *a;
    *a = *b;
    *b = temp;
}

typedef struct {
    int *arr;
    int low;
    int high;
} ThreadArgs;


int partition(int *arr, int low, int high) {
    int pivot = arr[high]; // Pivot is the last element
    int i = low - 1;       // i tracks the boundary of the smaller zone

    for (int j = low; j < high; j++) {// If element is smaller than pivot then i is incremented and then two values are swapped
        if (arr[j] < pivot) {
            i++; 
            swap(&arr[i], &arr[j]);
        }
    }
    
    // Move the pivot to the correct spot (right after the smaller zone)
    swap(&arr[i + 1], &arr[high]);
    
    // Return the index where the pivot now sits
    return (i + 1);
}

void quicksort(int *arr, int low, int high) {
    if (low < high) {
        int pivot = partition(arr, low, high);
        if (high - low > 1) {
            #pragma omp task firstprivate(arr, low, pivot)
            quicksort(arr, low, pivot - 1);

            #pragma omp task firstprivate(arr, high, pivot)
            quicksort(arr, pivot + 1, high);
        } else {
            // Sequential for small partitions
            quicksort(arr, low, pivot - 1);
            quicksort(arr, pivot + 1, high);
        }
    }
}
int main(int argc, char *argv[]) {
   
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <number_of_elements>\n", argv[0]);
        return 1;
    }
    int n = atoi(argv[1]);
   
    int *arr = malloc(n * sizeof(int)); //allocate array on the heap
    if (arr == NULL) {
        perror("Failed to allocate memory");
        return 1;
    }

    srand(time(NULL)); //seed srand and fill array
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % (2 * n); // Random numbers between 0 - 2n
    }

    struct timeval start, end; // start timing
    gettimeofday(&start, NULL);

   #pragma omp parallel
    {
        #pragma omp single
        quicksort(arr, 0, n - 1);
    }

    gettimeofday(&end, NULL); //stop timer
    double time_taken = (end.tv_sec - start.tv_sec) + 
                        (end.tv_usec - start.tv_usec) / 1e6;

    printf("Sorted in %f seconds.\n", time_taken);

    // Clean up
    free(arr);
    return 0;
}
