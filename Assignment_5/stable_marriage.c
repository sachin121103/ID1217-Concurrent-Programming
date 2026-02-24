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
            printf("[Man %d]'s shot went in! Accepted by [Woman %d]\n", man_id+1, status.MPI_SOURCE - n + 1);
            fflush(stdout);
            break;
        
        case TAG_REJECT:
            printf("Oh my goodness Valentine! [Man %d] was rejected by [Woman %d]\n", man_id+1, status.MPI_SOURCE - n + 1);
            fflush(stdout);
            break;
        
        case TAG_DUMP:
            engaged = 0;
            current_partner = -1;
            printf("[Man %d] dumped [Woman %d]. Back to the drawing board buddy.\n", man_id+1, status.MPI_SOURCE - n + 1);
            fflush(stdout);
            break;

        case TAG_STOP:
            running = 0;
            if (engaged)
            {
                printf("[Man %d] cuffed with [Woman %d]. Good job!\n", man_id+1, current_partner - n +1);
            } else
            {
                printf("[Man %d] ended single. Time to hit the bottle\n", man_id+1);
            }
            fflush(stdout);
            break;
        
        default:
            break;
        }
    }
    
}

void run_woman(int rank, int n, int preferences[]){

int current_man_id = -1;
int current_man_rank = -1;

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

void run_counter(int n){

    int matched_women_count = 0;
    int signal_buffer;
    MPI_Status status;

        while (matched_women_count < n) {
        
            MPI_Recv(&signal_buffer, 1, MPI_INT, MPI_ANY_SOURCE, TAG_MATCHED, MPI_COMM_WORLD, &status);

            matched_women_count++;

            printf("Notification received from Woman %d. Total matched: %d/%d\n", 
                status.MPI_SOURCE, matched_women_count, n);
        }

        for (int i = 1; i < 2*n; i++) { //stop everyone
            MPI_Send(NULL, 0, MPI_INT, i, TAG_STOP, MPI_COMM_WORLD);
        }

    printf("Everyone in stable marriages");
}

void generate_preferences(int *prefs, int n){
    for (int i = 0; i < n; i++)
    {
        prefs[i] = i;
    }

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

    if (rank < n)
    {
        run_man(rank, n, preferences);
    } else if (rank < 2*n)
    {
       run_woman(rank, n, preferences);
    } else
    {
        run_counter(n);
    }

    MPI_Finalize();
    return 0;

}
