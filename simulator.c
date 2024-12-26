#include "simulator.h"
#include "list.h"
#include "non_blocking_queue.h"
#include "blocking_queue.h"
#include "utilities.h"
#include "logger.h"
#include <string.h>

static int* thread_ids;
static pthread_t* threads;
static BlockingQueueT* pid_queue; //stores all initial max number of pids
static BlockingQueueT* ready_queue; //stores all initialised process pids
static int count; //thread_count
static ProcessT* process_table; //process table holds all process structs

void simulator_start(int thread_count, int max_processes) {
  
  count = thread_count;
  
  //init process table
  process_table = (ProcessT*)checked_malloc(max_processes * sizeof(ProcessT));
  memset(process_table, 0, max_processes * sizeof(ProcessT)); // clean slate
  
  //init blocking queues
  pid_queue = (BlockingQueueT*)checked_malloc( sizeof(BlockingQueueT) );
  ready_queue = (BlockingQueueT*)checked_malloc( sizeof(BlockingQueueT) );
  blocking_queue_create(pid_queue);
  blocking_queue_create(ready_queue);
  
  //unique thread ids
  threads = (pthread_t*)checked_malloc(thread_count * sizeof(pthread_t));
  thread_ids = (int*)checked_malloc(thread_count * sizeof(int));
  
  for(int i = 0; i<thread_count ; i++){
    //save thread id into array
    thread_ids[i] = i+1;
    pthread_create( (&threads[i]), NULL, simulator_routine, &thread_ids[i]);
  }
  
  //populate queue with available process ids
  for(unsigned int i = 0; i<max_processes ; i++){
    ProcessIdT process_id = i+1;
    blocking_queue_push(pid_queue,process_id);
  }
  
}

void* simulator_routine(void *arg){
  //retrieve identifier
  int thread_id = *(int*)arg;
  char message[50];
  //formatting for logger
  sprintf(message , "Thread %i has started" , thread_id);
  logger_write(message);
  
  while(1){
    ProcessIdT pid;
    
    if(ready_queue->terminated == 1){
      break; //exit loop if queue is terminated
    }
    
    //pop ready queue
    if (blocking_queue_pop(ready_queue,&pid) != 0){
      continue;
    }
    
    //fetch process struct
    ProcessT* process = &process_table[pid - 1];
    
    // run the process 
    EvaluatorResultT result = evaluator_evaluate(process->eval_code, process->pc);
    
    if(result.reason == reason_terminated){
      //process done
      sem_post(&process->semaphore);
    }else if (result.reason == reason_timeslice_ended) {
      //timeslice ended
      process->pc = result.PC;
      blocking_queue_push(ready_queue, pid);
    }
    
  }
   
  //finish thread
  return NULL;
}

ProcessIdT request_process(){
  //returns first ID in the queue
  ProcessIdT process;
  blocking_queue_pop(ready_queue , &process);
  return process;
}

void return_process(ProcessIdT process){
  blocking_queue_push(pid_queue,process);
}

void simulator_stop() {
  //join each thread
  for(int i=0; i<count; i++){
    pthread_join(threads[i],NULL);
  }
  
  blocking_queue_destroy(pid_queue);
  blocking_queue_destroy(ready_queue);
   //add after implementation
   
  //checked_free(pid_queue);
  //checked_free(ready_queue);
  
  // Clean up allocated memory
  checked_free(threads);
  checked_free(thread_ids);
  checked_free(process_table);
 
  threads = NULL;
  thread_ids = NULL;
  
}

ProcessIdT simulator_create_process(EvaluatorCodeT const code) {
  ProcessIdT pid;
  
  if(blocking_queue_pop(pid_queue,&pid) != 0){
    return 0;
  }
  
  //save info about process
  ProcessT* process = &process_table[pid - 1]; 
  process->pid = pid;
  process->eval_code = code;
  process->pc = 0;
  process->completed = 0;
  
  // init the semaphore
  sem_init(&process->semaphore, 0, 0);
  
  //add initialised process id to ready queue
  blocking_queue_push(ready_queue, pid);
  
  char message[50];
  //formatting for logger
  sprintf(message , "Process %i has been created" , pid);
  logger_write(message);
  
  return pid;
}

void simulator_wait(ProcessIdT pid) {
  ProcessT* process = &process_table[pid - 1];
  
  // Log that we are waiting for the process
  char log_msg[50];
  sprintf(log_msg, "Waiting for process %u to finish", pid);
  logger_write(log_msg);
  
  //wait for process to finish
  sem_wait(&process->semaphore);
  
  //free semaphore
  sem_destroy(&process->semaphore);
  
  //clear entry in process table
  memset(process, 0, sizeof(ProcessT));
  
  //reuse pid by adding back to pid queue
  blocking_queue_push(pid_queue, pid);
  
}

void simulator_kill(ProcessIdT pid) {
}

void simulator_event() {
}
