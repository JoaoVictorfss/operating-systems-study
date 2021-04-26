/**
 * 
 * @author João Victor Fernandes de Souza Silva 
 * @date 24/04/2021 
 *  
 * */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include "queue.h"
#include "statistics.h"


#define KEY 8000
#define MEM_SZ 3300
#define STOP 1000
#define NUM_PROCESS 7

//P7
#define NUM_THREADS_P7 3

//P4
#define NUM_THREADS_P4 2
#define P4_THREAD01 1
#define P4_THREAD02 2

//busy wait
#define P5_ID 0
#define P6_ID 1
#define P7_ID 2


struct shared_area{
  int writeFlag, readFlag;
  int p5ProcessedQtt, p6ProcessedQtt;
  int *p5Pipe, *p6Pipe;
  int pid04, num;
  Queue f1, f2;
  sem_t p123Mutex, p4ThreadsMutex;
};

struct final_data{
  int count;
  int array[STOP];
  clock_t t;
};

struct shared_area *shared_area_ptr;

void showFeedBack(struct final_data final_data, Fashion fashion) {
  final_data.t = clock() - final_data.t; 

  printf("\n_____________________________________________________________________________\n\n");
  printf("Tempo de execucao: %lfms.\n", ((double)final_data.t) / ((CLOCKS_PER_SEC / 1000)));
  printf("Quantidade de valores processados por P5 e P6 respectivamente: %d %d\n", shared_area_ptr->p5ProcessedQtt, shared_area_ptr->p6ProcessedQtt);
  printf("Maior Valor: %d.\n", final_data.array[STOP - 1]);
  printf("Menor Valor: %d.\n", final_data.array[0]);
  printf("Moda: %d, repetindo %d vezes.\n", fashion.value, fashion.amount);
  printf("_____________________________________________________________________________\n\n");
}

int previous = P5_ID;

void *consumeFromQueueF2(void *args) {
  struct final_data *final_data = (struct final_data *) args;
  
  while (1) {
   if (shared_area_ptr->num == P7_ID){
        if (final_data->count == STOP){
          break;
        }
      
        if(!queue_empty(&shared_area_ptr->f2)) {
          int value = queue_get(&shared_area_ptr->f2);
          queue_pop(&shared_area_ptr->f2);
            if(value >= 1){
              final_data->array[final_data->count] = value;
              final_data->count++;
              printf("%d\n", value);
            }
        }
        if (previous == P5_ID) shared_area_ptr->num = P6_ID;
        else shared_area_ptr->num = P5_ID;
        previous = shared_area_ptr->num;
    }
  }
}

void consumingF2AndShowFeedback(struct final_data final_data) {
  pthread_t threads[NUM_THREADS_P7];
  Fashion fashion;

  for (int i = 0; i < NUM_THREADS_P7 -1; i++) {
    if (pthread_create(&threads[i], NULL, consumeFromQueueF2, &final_data))  exit(-1);
  }
  consumeFromQueueF2(&final_data);

  rol(final_data.array, 0, STOP - 1);
  fashion = c_fashion(final_data.array, STOP);

  showFeedBack(final_data, fashion);

  exit(0);
}

void processP5Flux() {
  while (1) {
    if(shared_area_ptr->num == P5_ID){
      if (queue_full(&shared_area_ptr->f2)){
        shared_area_ptr->num = P7_ID;
      }else{
        int value = 0;

        if(read(shared_area_ptr->p5Pipe[0], &value, sizeof(int)) > -1){
            queue_push(&shared_area_ptr->f2, value);
            shared_area_ptr->p5ProcessedQtt++;
            shared_area_ptr->num = P7_ID;
        }else{
            shared_area_ptr->num = P6_ID;
        }
      }
    }
  }
}

void processP6Flux() {
    while (1) {
      if(shared_area_ptr->num == P6_ID){
        if(queue_full(&shared_area_ptr->f2)) {
          shared_area_ptr->num = P7_ID;
        }else{
          int value = 0;
          
          if(read(shared_area_ptr->p6Pipe[0], &value, sizeof(int)) > -1){
              queue_push(&shared_area_ptr->f2, value);
              shared_area_ptr->p6ProcessedQtt++;

              shared_area_ptr->num = P7_ID;
          }else{
              shared_area_ptr->num = P5_ID;
          }
        }
    }
  }
}

void *consumeFromQueueF1AndSend(void *args) {
  int *whoAmI = (int *) args;

  while (1) {
    sem_wait((sem_t *)&shared_area_ptr->p4ThreadsMutex);
    if (shared_area_ptr->readFlag) {
      if (queue_empty(&shared_area_ptr->f1)) {
        shared_area_ptr->readFlag = 0;
        shared_area_ptr->writeFlag = 1;
        sem_post((sem_t *)&shared_area_ptr->p4ThreadsMutex);
      }else{
        int sendingValue = queue_get(&shared_area_ptr->f1);
        queue_pop(&shared_area_ptr->f1);

        if (*whoAmI == P4_THREAD01){
          write(shared_area_ptr->p5Pipe[1], &sendingValue, sizeof(int));
        }else{
          write(shared_area_ptr->p6Pipe[1], &sendingValue, sizeof(int));
        }
      }
    }
    sem_post((sem_t *)&shared_area_ptr->p4ThreadsMutex);
  }
}

void processP4Flux() {
  shared_area_ptr->pid04 = getpid();
  pthread_t p4_thread02;
  int p4T1 = P4_THREAD01;
  int p4T2 = P4_THREAD02;

  if (sem_init((sem_t *)&shared_area_ptr->p4ThreadsMutex, 0, 1) != 0)
    exit(-1);

  pthread_create(&p4_thread02, NULL, consumeFromQueueF1AndSend, &p4T1); 
  consumeFromQueueF1AndSend(&p4T2);
}

void setupReadFlag(int sig) {
  shared_area_ptr->readFlag = 1;
}

void startProducing() {
  while (1) {
    sem_wait((sem_t *)&shared_area_ptr->p123Mutex);
    if (shared_area_ptr->writeFlag) {
      if (!queue_full(&shared_area_ptr->f1)) {
        int newValue = rand() % 1000 + 1;
        queue_push(&shared_area_ptr->f1, newValue);
       
        if (queue_full(&shared_area_ptr->f1)) {
          shared_area_ptr->writeFlag = 0;
          kill(shared_area_ptr->pid04, SIGUSR1);
        }
      }
    }
    sem_post((sem_t *)&shared_area_ptr->p123Mutex);
  }
}

void initData(int *p5Pipe, int *p6Pipe, struct final_data *final_data) {
  shared_area_ptr->writeFlag = 1;
  shared_area_ptr->readFlag = 0;
  shared_area_ptr->p5Pipe = p5Pipe;
  shared_area_ptr->p6Pipe = p6Pipe;
  shared_area_ptr->p5ProcessedQtt = 0;
  shared_area_ptr->p6ProcessedQtt = 0;
  shared_area_ptr->num = -2;
  final_data->count = 0;
  queue_create(&shared_area_ptr->f1);
  queue_create(&shared_area_ptr->f2);
}

void createSharedMemory() { 
  int shmid;
  key_t key = KEY;

  void *shared_memory = (void *)0;

  shmid = shmget(key, MEM_SZ, 0666 | IPC_CREAT);
  if (shmid == -1)
    exit(-1);

  shared_memory = shmat(shmid, (void *)0, 0);
  if (shared_memory == (void *)-1)
    exit(-1);

  shared_area_ptr = (struct shared_area *)shared_memory;
}

int main() {
  int pids[7], status[7], pid;
  struct final_data final_data;

  final_data.t = clock();

  createSharedMemory();

  int p5Pipe[2];
  if (pipe(p5Pipe) == -1) return -1;

  int p6Pipe[2];
  if (pipe(p6Pipe) == -1) return -1;

  //semáforo de P1, P2 e P3
  if (sem_init((sem_t *)&shared_area_ptr->p123Mutex, 1, 0) != 0) exit(-1);

  initData(p5Pipe, p6Pipe, &final_data);

  for (int i = 0; i < NUM_PROCESS; i++) {
    pid = fork();
    if (pid > 0) { //processo pai
      pids[i] = pid;
    }
    else {
      if (i == 3) { //P4
        signal(SIGUSR1, setupReadFlag);
        processP4Flux();
        while (1){}
      } else if (i < 3 && i >= 0){//P1 || P2 || P3
         startProducing();
      }else {
         if (i == 4) { // P5
           processP5Flux();
        }if (i == 5) { // P6
           processP6Flux();
        } if (i == 6) { // P7
           consumingF2AndShowFeedback(final_data);
        }
      }
      break;
    }
 }
 
  if (pid > 0) { //processo pai
    int status;

    //libera semáforo de p1, p2 e p3
    sem_post(&shared_area_ptr->p123Mutex);

    sleep(1);

    //inicializa P5
    shared_area_ptr->num = P5_ID;

    waitpid(pids[6], &status, WUNTRACED);
    for (int i = 0; i < 6; i++){
      kill(pids[i], SIGKILL);
    }
    close(shared_area_ptr->p5Pipe[1]);
    close(shared_area_ptr->p6Pipe[1]);

    return 0;
  }
}