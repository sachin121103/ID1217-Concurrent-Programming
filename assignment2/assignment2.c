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

void* parallel_quicksort(void* arg) {

    ThreadArgs* data = (ThreadArgs*)arg; //unpack struct
    int *arr = data->arr;
    int low = data->low;
    int high = data->high;

    if (low < high) {



        int pivot = partition(arr, low, high);


        if (high - low > 1000){

            pthread_t thread;
                    
            ThreadArgs* left_args = malloc(sizeof(ThreadArgs)); //arguments for smaller array
            left_args->arr = arr;
            left_args->low = low;
            left_args->high = pivot - 1;

            
            pthread_create(&thread, NULL, parallel_quicksort, left_args); // start threading left


            ThreadArgs right_args = {arr, pivot + 1, high}; // bigger array side args
            parallel_quicksort(&right_args); // parent process 

            
            pthread_join(thread, NULL); // join both threads
            free(left_args); // free memory we allocated
            
        }
        else{ // sort sequentially

            ThreadArgs left_args = {arr, low, pivot - 1};
            ThreadArgs right_args = {arr, pivot + 1, high};
            parallel_quicksort(&left_args);
            parallel_quicksort(&right_args);
        }
    }
    return NULL;
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

    ThreadArgs args = {arr, 0, n - 1};
    parallel_quicksort(&args); //sort

    gettimeofday(&end, NULL); //stop timer
    double time_taken = (end.tv_sec - start.tv_sec) + 
                        (end.tv_usec - start.tv_usec) / 1e6;

    printf("Sorted in %f seconds.\n", time_taken);

    // Clean up
    free(arr);
    return 0;
}
