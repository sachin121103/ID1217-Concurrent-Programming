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

int current_man_id = -1;
int current_man_rank = -1

MPI_status status;
int new_man;


while (true){

    MPI_Recv(&incoming_man, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    switch(status.MPI_TAG){

        case TAG_STOP:
            printf("[Woman %d] Locked in 4lyff: Man %d\n", my_id, current_partner);
            break;

        case TAG_PROPOSE:
            int new_man = man_id;
            int new_rank = rank[incoming_man];
        
            if(current_man_id == -1){ //accepts directly cause single
                current_man_id = new_man;
                current_man_rank = new_rank;
                MPI_Send(NULL, 0, MPI_INT, incoming_man, TAG_ACCEPT, MPI_COMM_WORLD);
                printf("[Woman %d] Accepted my Mannnnnnn %d (First proposal)\n", my_id, incoming_man);
            }

            if(new_rank < current_man_rank){ //rejects directly cause new man worse
                printf("[Woman %d] REJECTED this bitchass Man %d (Current partner %d is better)\n", my_id, incoming_man, current_partner);
                MPI_Send(NULL, 0, MPI_INT, incoming_man, TAG_REJECT, MPI_COMM_WORLD);
            }

            if(new_rank > current_man_rank){ //dumps old man cause new man better

                printf("[Woman %d] DUMPING that bitchass Man %d for BETTER Mannnn %d\n", my_id, current_partner, incoming_man);
                    
                    MPI_Send(NULL, 0, MPI_INT, current_partner, TAG_DUMP, MPI_COMM_WORLD);
            
                    current_partner = incoming_man;
                    current_rank = new_rank;
                    MPI_Send(NULL, 0, MPI_INT, incoming_man, TAG_ACCEPT, MPI_COMM_WORLD);
            }

        break;
    }
}



    








    }





}







void run_counter(int n);

int main(int argc, char** argv);