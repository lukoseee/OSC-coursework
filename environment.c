#include "environment.h"
#include "simulator.h"
#include "utilities.h"
#include "evaluator.h"
#include "list.h"
#include <stdio.h>

static pthread_t* threads;
static pthread_t* blocking_threads;
static pthread_t* infinite_threads;
static int* thread_ids;
static int* blocking_thread_ids;
static int* infinite_thread_ids;
static int count; 
static int iters;
static int batch;


void *terminating_routine(void *arg){
  //retrive thread id
  int thread_id = *(int*)arg;
  
  EvaluatorCodeT const code = evaluator_terminates_after(5);
  
  for(int i=0; i<iters; i++){ //loop through iterations
    
    ProcessIdT pids[batch];
    
    for(int y=0; y<batch; y++){ //loop through batch

      pids[y] = simulator_create_process(code);
      
    }
    
    //iterate through processes and wait for termination
    for(int j = 0 ; j<batch;j++)
    { 
       if(pids[j] > 0){
       
        simulator_wait(pids[j]);
       }
       
    }
  }
  
  //printf("thread exiting\n");
  return NULL; //exit thread
}

void *blocking_routine(void *arg){
  //retrive thread id
  int thread_id = *(int*)arg;
  
  EvaluatorCodeT const code = evaluator_blocking_terminates_after(5);
  
  for(int i=0; i<iters; i++){ //loop through iterations
    
    ProcessIdT pids[batch];
    
    for(int y=0; y<batch; y++){ //loop through batch

      pids[y] = simulator_create_process(code);
      
    }
    
    //iterate through processes and wait for termination
    for(int j = 0 ; j<batch;j++)
    { 
       if(pids[j] > 0){
       
        simulator_wait(pids[j]);
       }
       
    }
  }
  
  //printf("thread exiting\n");
  return NULL; //exit thread
}

void *infinite_routine(void *arg){
  //retrive thread id
  int thread_id = *(int*)arg;
  
  EvaluatorCodeT const code = evaluator_infinite_loop;
  
  for(int i=0; i<iters; i++){ //loop through iterations
    
    ProcessIdT pids[batch];
    
    for(int y=0; y<batch; y++){ //loop through batch

      pids[y] = simulator_create_process(code);
      
    }
    
    for(int y=0; y<batch; y++){ //loop through batch

      simulator_kill(pids[y]);
      
    }
    
    //iterate through processes and wait for termination
    for(int j = 0 ; j<batch;j++)
    { 
       if(pids[j] > 0){
       
        simulator_wait(pids[j]);
       }
       
    }
  }
  
  //printf("thread exiting\n");
  return NULL; //exit thread
}



void environment_start(unsigned int thread_count, unsigned int iterations,unsigned int batch_size) {

  count = thread_count;
  iters = iterations;
  batch = batch_size;
  threads = (pthread_t*)checked_malloc(thread_count * sizeof(pthread_t));
  blocking_threads = (pthread_t*)checked_malloc(thread_count * sizeof(pthread_t));
  infinite_threads = (pthread_t*)checked_malloc(thread_count * sizeof(pthread_t));
  thread_ids = (int*)checked_malloc(thread_count * sizeof(int));
  blocking_thread_ids = (int*)checked_malloc(thread_count * sizeof(int));
  infinite_thread_ids = (int*)checked_malloc(thread_count * sizeof(int));
  		       
  for(int i = 0; i<thread_count ; i++){
    //save thread id into array
    thread_ids[i] = i+1;
    blocking_thread_ids[i] = i+1;
    infinite_thread_ids[i] = i+1;
    pthread_create( (&threads[i]), NULL, terminating_routine, &thread_ids[i] );
    pthread_create( (&blocking_threads[i]), NULL, blocking_routine, &blocking_thread_ids[i]);
    pthread_create( (&infinite_threads[i]), NULL, blocking_routine, &infinite_thread_ids[i]);
  }
}

void environment_stop() {
  //join each thread
  for(int i=0; i<count; i++){
    if (pthread_join(threads[i], NULL) != 0) {
        perror("Failed to join terminating thread");
    }

    if (pthread_join(blocking_threads[i], NULL) != 0) {
        perror("Failed to join blocking thread");
    }
    
    if (pthread_join(infinite_threads[i], NULL) != 0) {
        perror("Failed to join blocking thread");
    }
  }
 
  checked_free(threads);
  checked_free(blocking_threads);
  checked_free(thread_ids);
  checked_free(blocking_thread_ids);
  checked_free(infinite_thread_ids);
  checked_free(infinite_threads);
  
  threads = NULL;
  thread_ids = NULL;
  blocking_threads = NULL;
  infinite_threads = NULL;
  infinite_thread_ids = NULL;
}
