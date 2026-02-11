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

#define MAXBEES 1000
#define MAX_H 10000000

int honey_count = 0;
int H;
sem_t *mutex;
sem_t *bear_sleep;
int NUM_BEES;

void* bear_function(void* arg){
    while (1)
    {
        sem_wait(bear_sleep);
        sem_wait(mutex);
        printf("Bear is eating the honey\n");
        fflush(stdout);
        honey_count = 0;
        sem_post(mutex);
    }

    return NULL;
}

void* bee_function(void* arg){
    int id = (int)(long)arg;
    while (1)
    {
        usleep(100000);
        sem_wait(mutex);
        honey_count += 1;
        printf("Bee %d dropped one unit of honey\n", id);
        if (honey_count == H)
        {
            printf("Pot is full\n");
            sem_post(bear_sleep);
            sem_post(mutex);

        }
        else
        {
            sem_post(mutex);
        }
    }
    return NULL;
}


int main(int argc, char *argv[]){

    H = (argc > 1)? atoi(argv[1]) : MAX_H;
    NUM_BEES = (argc > 2)? atoi(argv[2]) : MAXBEES;

    pthread_t bees[NUM_BEES];
    pthread_t bear_id;

    mutex = sem_open("/bee_mutex", O_CREAT, 0644, 1);
    bear_sleep = sem_open("/bear_sleep", O_CREAT, 0644, 0); // 0644 is permission, O_CREAT is to make it

    sem_unlink("/bee_mutex");
    sem_unlink("/bear_sleep");

    pthread_create(&bear_id, NULL, bear_function, NULL);
    

    for (int i = 0; i < NUM_BEES; i++)
    {
       pthread_create(&bees[i], NULL, bee_function, (void*)(long)i);
    }

    pthread_join(bear_id, NULL);

    return 0;
}