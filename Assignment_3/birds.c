#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <limits.h>
#include <semaphore.h>

#define MAX_BABIES 1000
#define DEFAULT_W 10

int worms_count = 0;
int W; 
int NUM_BABIES;

sem_t *mutex;
sem_t *parent_sleep;
sem_t *dish_full;

void* parent_bird_function(void* arg) {
    while (1) {
        sem_wait(parent_sleep);  // Wait for a baby to chirp (empty dish)
        
        printf("\nParent wakes up & gathers %d worms \n", W);
        fflush(stdout);
        
        sem_wait(mutex);
        worms_count = W;
        sem_post(mutex);
        
        printf("\nParent wakes up & gathers %d worms \n");
        fflush(stdout);
        
        // Signal the waiting baby that the dish is ready
        sem_post(dish_full);
    }
    return NULL;
}

void* baby_bird_function(void* arg) {
    int id = (int)(long)arg;
    while (1) {
        // Sleep/Rest for a bit before trying to eat
        srand(time(NULL));
        usleep(rand() % 500000); 

        sem_wait(mutex);
        
        if (worms_count == 0) {
            printf("Baby %d: chirps loudly to awaken the parent bird\n", id);
            fflush(stdout);
            
            sem_post(parent_sleep); // Wake parent
            sem_post(mutex);        // Release mutex so parent can refill
            
            sem_wait(dish_full);    // Wait for refill signal
            sem_wait(mutex);        // Re-acquire mutex to take a worm
        }

        worms_count--;
        printf("Baby %d took a worm. Worms remaining: %d\n", id, worms_count);
        fflush(stdout);

        sem_post(mutex);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    // Parse arguments: ./birds <num_babies> <dish_capacity>
    NUM_BABIES = (argc > 1) ? atoi(argv[1]) : 5;
    W = (argc > 2) ? atoi(argv[2]) : DEFAULT_W;

    pthread_t babies[NUM_BABIES];
    pthread_t parent_id;

    // Use semaphores
    mutex = sem_open("/bird_mutex", O_CREAT, 0644, 1);
    parent_sleep = sem_open("/parent_sleep", O_CREAT, 0644, 0);
    dish_full = sem_open("/dish_full", O_CREAT, 0644, 0);

    // Unlink immediately so they are destroyed when the program exits
    sem_unlink("/bird_mutex");
    sem_unlink("/parent_sleep");
    sem_unlink("/dish_full");

    // Initialize dish as empty to start the cycle
    worms_count = 0;

    pthread_create(&parent_id, NULL, parent_bird_function, NULL);

    for (int i = 0; i < NUM_BABIES; i++) {
        pthread_create(&babies[i], NULL, baby_bird_function, (void*)(long)i);
    }

    // Join the parent 
    pthread_join(parent_id, NULL);

    return 0;
}
