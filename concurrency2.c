#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include<semaphore.h>

typedef struct my_philosopher_setup{
    int position;
    int count;
    sem_t *get_forks;
    sem_t *is_locked;
}params_t;


void initialize_semaphores(sem_t *is_locked, sem_t *get_forks, int fork_amount){
    int i=0;
    for(i;i<num_forks;i++){
        sem_init(&get_forks[i],0,1);
    }

    sem_init(lock, 0, fork_amount-1);
}

void run_all_threads(pthread_t *threads, sem_t *get_forks
