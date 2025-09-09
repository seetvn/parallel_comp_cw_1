#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include<stdbool.h>
#include <sys/time.h>
#include "helper_func.h"

/* -------shared variables-------- */
double **DATA;
double **COPY;
int *PRECISION_ARRAY;
bool STOP_FLAG = false;
pthread_barrier_t barrier;
int DIM = 6;
int DEBUGGER_MODE = 0;
int ITERATION = 0;
int PRINT_FINAL_ARRAYS = 1;
double PRECISION = 0.1;
int NOT_CONVERGED; //formally run
int NUM_THREADS = 3;
/* ------end of shared variables-------*/


/* ----- struct for worker threads ------*/
struct arrangement {
  int start;
  int end;
};

struct information {
  int size;
  int thread_id;
  struct arrangement start_end;
};
/* -------- end of struct-- ---------*/



void *setter(void *arg) {
  /* ---getting information from struct ------*/
  struct information *thread_info = (struct information *)arg;
  int thread_id = thread_info->thread_id;
  int start_row = thread_info->start_end.start;
  int end_row = thread_info->start_end.end;
  /* --- end of getting information from struct -----*/
  
  int gc = (end_row - start_row) * (DIM - 2);
  int converged_count;
  int iteration = 0;
  double newValue;
  double currentValue;
  double diff;
  while (NOT_CONVERGED) {
     converged_count = 0;
      for (int i = start_row; i < end_row; ++i) {
          for (int j = 1; j < DIM - 1; ++j) {
            /* --- updating value-------*/
              currentValue = DATA[i][j];
              newValue = (DATA[i][j + 1] +DATA[i + 1][j] +DATA[i][j - 1] +DATA[i - 1][j]) / 4;
              COPY[i][j] = newValue;
            /* --- end of  updating value-------*/
            
              diff = fabs(newValue - currentValue);
              if (diff < PRECISION) {
                converged_count++; 
              }
          }
      }
       if  (converged_count == gc){
          PRECISION_ARRAY[thread_id] = 1;
       }
    /* ----------------DEBUG message ---------------------*/
       if (DEBUGGER_MODE){printf("-----------Thread: %d has finished its %d iteration,convergence is %d----------\n",thread_id,iteration,converged_count == gc);printf("\n");}
    /* ----------------END OF DEBUG message ---------------------*/
    
        iteration++;
        pthread_barrier_wait(&barrier); // wait for all co-setter threads to be finished 

        pthread_barrier_wait(&barrier);// wait for checker(main) thread to be finished
    };
    return NULL;
}

int main(int argc, char *argv[]) {
  printf("test");
   
/* -----------variable initilzation -----------*/
    DIM = (int) strtol(argv[1], NULL, 10);
    NUM_THREADS = atoi(argv[2]);
    PRECISION = atof(argv[3]);
    DEBUGGER_MODE = atoi(argv[4]);
    PRINT_FINAL_ARRAYS = atoi(argv[5]);
  
    DATA = create_array(DIM,DIM);
    COPY = create_array(DIM,DIM);
    PRECISION_ARRAY = calloc(NUM_THREADS , sizeof(int));
    NOT_CONVERGED = 1;
    struct timeval _start; 
    struct timeval seqEnd, paraEnd, seqDuraton, paraDuration;
  /* -----------end of variable initilzation -----------*/

  
  /*---- timing ----------*/
    gettimeofday(&_start, NULL);

    
  /*------- assigning rows for each thredad----------*/
   int remainder = (DIM - 2) % NUM_THREADS;
   int initial_rows_per_thread = (DIM- 2) / NUM_THREADS;
   int* start_row = malloc(NUM_THREADS * sizeof(int));
   int* end_row = malloc(NUM_THREADS * sizeof(int));
   int start_thread = 1;
   int end_thread;
   for (int i = 0; i < NUM_THREADS; ++i) {
     end_thread = start_thread + initial_rows_per_thread;
     if (i < remainder) {end_thread++;}
     start_row[i] = start_thread;
     end_row[i] = end_thread;
     start_thread = end_thread;
   }
  /* -------------end of assigning rows-----------------*/


  /* ---DEBUG MESSAGE-----*/
   if (DEBUGGER_MODE) {for (int i = 0;i<NUM_THREADS;i++){printf("thread %d start: %d, end: %d for %d rows \n",i,start_row[i],end_row[i],end_row[i]-start_row[i]);}}
  /* --- end of debug message-----*/

  
  /* -----------intiialising setter threads + barrier ----------- */
    struct information thread_info_array[NUM_THREADS];
    pthread_t threads[NUM_THREADS];
    pthread_barrier_init(&barrier, NULL, NUM_THREADS + 1);
    for (int i = 0; i < NUM_THREADS; ++i) {
        struct information info;
        info.size = DIM;
        info.thread_id = i;
        info.start_end.start = start_row[i];
        info.start_end.end = end_row[i];
        thread_info_array[i] = info;
        if (pthread_create(&threads[i], NULL, setter,&thread_info_array[i])) {
            exit(1);
        }
    }
  /* -----------end of intiialising setter threads + barrier----------- */
  

  /*---- timing ----------*/
    gettimeofday(&seqEnd, NULL);

  /* --- checker thread to check if converged------- */
    while (NOT_CONVERGED == 1) {
        pthread_barrier_wait(&barrier);
        swap(&DATA, &COPY);
        NOT_CONVERGED = 0;
        for (int i = 0; i < NUM_THREADS;i++) {
            if (PRECISION_ARRAY[i] == 0) {
                NOT_CONVERGED = 1;
                break;
            }
        }
        pthread_barrier_wait(&barrier);
    }

  /* ---- wait for all threads to end -------------*/
    for (int i = 0; i < NUM_THREADS;i++) {
        pthread_join(threads[i], NULL);
    }

  /* ---destroy barrier --------*/
  pthread_barrier_destroy(&barrier);

  /* --------time function----------*/
    gettimeofday(&paraEnd, NULL);
    timersub(&seqEnd, &_start, &seqDuraton);
    timersub(&paraEnd, &seqEnd, &paraDuration);

  
    printf("elpased time: %ld.%06ld, size: %d, number of Threads: %d\n", (long int)paraDuration.tv_sec, (long int)paraDuration.tv_usec, DIM,NUM_THREADS);

  /* ----------print final arrays---------------*/
  if (PRINT_FINAL_ARRAYS) {
    printf("---------------------DATA---------------------- \n");
    print_array(DATA, DIM, DIM);
    printf("---------------------COPY---------------------- \n");
    print_array(COPY, DIM, DIM);
    printf("%.lf \n",DATA[DIM-2][DIM-2]);
  }
  /* ---------------------end of print final arrays--------------*/

  /*----- free_memory --------*/
  free_array(DATA, DIM);
  free_array(COPY, DIM);
  free(PRECISION_ARRAY);
  free(start_row);
  free(end_row);
  printf("memory deallocated \n");
  /* -------end of free_memory---------*/
    

  return 1;
}
