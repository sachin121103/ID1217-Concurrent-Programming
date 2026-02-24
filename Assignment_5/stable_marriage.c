#include <stdio.h>
#include <mpi.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

// Message Tags
#define TAG_PROPOSE 1
#define TAG_ACCEPT  2
#define TAG_REJECT  3
#define TAG_DUMP    4
#define TAG_MATCHED 5
#define TAG_STOP    6

void run_man(int rank, int n, int preferences[]){
    int man_id = rank;
    int next_proposal = 0;
    int engaged = 0;
    int running = 1;
    int current_partner = -1; // Current fiancée's rank
    int message_buffer;
    
    MPI_Status status;

    while (running)
    {
        if (!engaged && next_proposal < n)
        {
            int woman_id = preferences[next_proposal];
            int woman_rank = n + woman_id;

            printf("[Man %d] Shooting like Curry from the moon to [Woman %d]\n", man_id+1, woman_id+1);
            fflush(stdout);

            MPI_Send(&man_id, 1, MPI_INT, woman_rank, TAG_PROPOSE, MPI_COMM_WORLD);
            next_proposal++;
        }

        MPI_Recv(&message_buffer, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        switch (status.MPI_TAG)
        {
        case TAG_ACCEPT:
            engaged = 1;
            current_partner = status.MPI_SOURCE;
            break;
        
        case TAG_REJECT:
            break;
        
        case TAG_DUMP:
            engaged = 0;
            current_partner = -1;
            break;

        case TAG_STOP:
            running = 0;
            break;
        
        default:
            break;
        }
    }
}

void run_woman(int rank, int n, int preferences[]){
    int my_id = rank - n;  // Woman's ID (0 to n-1)
    int current_partner = -1;
    int current_partner_rank = -1;
    int running = 1;
    int incoming_man;
    MPI_Status status;

    while (running){
        MPI_Recv(&incoming_man, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        switch(status.MPI_TAG){
            case TAG_STOP:
                running = 0;
                if (current_partner != -1) {
                    printf("[Woman %d] Locked in 4lyfe with Man %d\n", my_id + 1, current_partner + 1);
                } else {
                    printf("[Woman %d] STOP received. Ended single\n", my_id + 1);
                }
                fflush(stdout);
                break;

            case TAG_PROPOSE: {
                int new_man = incoming_man;
                
                // Find where this man ranks in preferences (0 = highest preference)
                int new_man_rank = -1;
                for (int i = 0; i < n; i++) {
                    if (preferences[i] == new_man) {
                        new_man_rank = i;
                        break;
                    }
                }
                
                if(current_partner == -1) { // Accepts directly because single
                    current_partner = new_man;
                    current_partner_rank = new_man_rank;
                    
                    MPI_Send(NULL, 0, MPI_INT, new_man, TAG_ACCEPT, MPI_COMM_WORLD);
                    
                    // Notify counter of first match
                    int counter_rank = 2 * n;
                    MPI_Send(&my_id, 1, MPI_INT, counter_rank, TAG_MATCHED, MPI_COMM_WORLD);
                    
                    printf("[Woman %d] Accepted my Mannnnnnn %d (First proposal)\n", my_id + 1, new_man + 1);
                    fflush(stdout);
                }
                else if(new_man_rank < current_partner_rank) { // New man is better (lower rank = higher preference)
                    printf("[Woman %d] DUMPING that Man %d for BETTER Man %d\n", 
                           my_id + 1, current_partner + 1, new_man + 1);
                    fflush(stdout);
                    
                    // Dump current partner
                    MPI_Send(NULL, 0, MPI_INT, current_partner, TAG_DUMP, MPI_COMM_WORLD);
                    
                    // Accept new man
                    current_partner = new_man;
                    current_partner_rank = new_man_rank;
                    MPI_Send(NULL, 0, MPI_INT, new_man, TAG_ACCEPT, MPI_COMM_WORLD);
                }
                else { // New man is worse - reject
                    printf("[Woman %d] REJECTED this Man %d (Current Man %d is better)\n", 
                           my_id + 1, new_man + 1, current_partner + 1);
                    fflush(stdout);
                    MPI_Send(NULL, 0, MPI_INT, new_man, TAG_REJECT, MPI_COMM_WORLD);
                }
                break;
            }
        }
    }
}

void run_counter(int n){
    int matched_women_count = 0;
    int signal_buffer;
    MPI_Status status;

    printf("[Counter] Starting. Waiting for all %d women...\n", n);
    fflush(stdout);

    while (matched_women_count < n) {
        MPI_Recv(&signal_buffer, 1, MPI_INT, MPI_ANY_SOURCE, TAG_MATCHED, MPI_COMM_WORLD, &status);
        matched_women_count++;
    }

    // Stop all men and women (processes 0 to 2n-1)
    for (int i = 0; i < 2 * n; i++) {
        MPI_Send(NULL, 0, MPI_INT, i, TAG_STOP, MPI_COMM_WORLD);
    }

    printf("Everyone in stable marriages\n");
    fflush(stdout);
}

void generate_preferences(int *prefs, int n){
    for (int i = 0; i < n; i++) {
        prefs[i] = i;
    }

    // Fisher-Yates shuffle
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = prefs[i];
        prefs[i] = prefs[j];
        prefs[j] = temp;
    }
}

int main(int argc, char** argv){
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int n = atoi(argv[1]);
    

    srand(time(NULL) + rank);

    int preferences[n];
    generate_preferences(preferences, n);

    if (rank < n) {
        run_man(rank, n, preferences);
    } else if (rank < 2 * n) {
        run_woman(rank, n, preferences);
    } else {
        run_counter(n);
    }

    MPI_Finalize();
    return 0;
}
