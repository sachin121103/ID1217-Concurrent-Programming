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




