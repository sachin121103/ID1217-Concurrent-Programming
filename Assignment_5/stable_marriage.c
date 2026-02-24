#include <stdio.h>
#include <mpi.h>
#include <stdbool.h>

// Message Tags
#define TAG_PROPOSE 1
#define TAG_ACCEPT  2
#define TAG_REJECT  3
#define TAG_DUMP    4
#define TAG_MATCHED 5
#define TAG_STOP    6





void run_man(int rank, int n, int preferences[]);

void run_woman(int rank, int n, int preferences[]);

bool engaged = false;
int current_man_id = -1;
int current_man_rank = -1

MPI_status status;
int new_man;


while (1){

    



}







void run_counter(int n);

int main(int argc, char** argv);