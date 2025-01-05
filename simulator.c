#include "simulator.h"
#include "list.h"
#include "non_blocking_queue.h"
#include "blocking_queue.h"
#include "utilities.h"
#include "logger.h"
#include "event_source.h"
#include <string.h>
#include <unistd.h>

static int* thread_ids;
static pthread_t* threads; //threads
static BlockingQueueT* pid_queue; //stores all initial max number of pids
static BlockingQueueT* ready_queue; //stores all initialised process pids
static NonBlockingQueueT* event_queue; 
static int count; //thread_count
static ProcessT* process_table; //process table holds all process structs
static pthread_mutex_t table_lock;
static int completed_process_count = 0;

void simulator_start(int thread_count, int max_processes) {
  
  count = thread_count;
  
  //init process table - array of processT structs
  process_table = (ProcessT*)checked_malloc(max_processes * sizeof(ProcessT));
  memset(process_table, 0, max_processes * sizeof(ProcessT)); // clean slate
  
  //init blocking queues
  pid_queue = (BlockingQueueT*)checked_malloc( sizeof(BlockingQueueT) );
  ready_queue = (BlockingQueueT*)checked_malloc( sizeof(BlockingQueueT) );
  event_queue = (NonBlockingQueueT*)checked_malloc( sizeof(NonBlockingQueueT) );
  
  //create each queue
  blocking_queue_create(pid_queue);
  blocking_queue_create(ready_queue);
  non_blocking_queue_create(event_queue);
  
  //init process table mutex
  pthread_mutex_init(&table_lock, NULL);
  
  //unique thread ids
  threads = (pthread_t*)checked_malloc(thread_count * sizeof(pthread_t));
  thread_ids = (int*)checked_malloc(thread_count * sizeof(int));
  
  for(int i = 0; i<thread_count ; i++){
    //save thread id into array
    thread_ids[i] = i+1; //plus 1 as ids start from 1
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
  
  char message[100];
  snprintf(message , sizeof(message), "Thread %i has started" , thread_id);
  logger_write(message);
  
  while(ready_queue->terminated == 0){ 
    ProcessIdT pid;
    
    //break if not popped
    if (blocking_queue_pop(ready_queue, &pid) != 0) {
      break;
    }

    //fetch process struct
    ProcessT* process = &process_table[pid - 1];
    
    //in the cases process entry has been erased
    if (process == NULL) {
      continue;
    }
    
    //skip process if terminated
    if(process->state == terminated){
      continue;
    }
    
    if (process->eval_code.implementation == NULL) {
      process->state = terminated;
      sem_post(&process->semaphore);
      continue;
    }
    
    //run the process 
    EvaluatorResultT result = evaluator_evaluate(process->eval_code, process->pc);
    
    if(result.reason == reason_terminated){
      //process finished
      process->completed = 1;
      process->state = terminated;
      sem_post(&process->semaphore);
      
    }else if (result.reason == reason_timeslice_ended) {
      //timeslice ended
      process->pc = result.PC; //update process structs pc
      process->state = ready;
      blocking_queue_push(ready_queue, pid); //push pid back to ready queue
    }
    else if(result.reason == reason_blocked){
      process->pc = result.PC; 
      process->state = blocked; //update state
      non_blocking_queue_push(event_queue, pid); //push to event queue
    }
  }
  
  //finish thread
  return NULL;
}

void simulator_stop() {
  
  //terminate queues before joining 
  blocking_queue_terminate(ready_queue);
  
  //post to semaphore so sim wait can finish
  sem_post(&ready_queue->queue_sem);
  blocking_queue_terminate(pid_queue);
  
  //join each thread
  for(int i=0; i<count; i++){
    pthread_join(threads[i], NULL);
  }
  
  //destroy and nullify queues
  blocking_queue_destroy(pid_queue);
  blocking_queue_destroy(ready_queue);
  non_blocking_queue_destroy(event_queue);
  
  // Clean up allocated memory
  checked_free(pid_queue);
  checked_free(ready_queue);
  checked_free(event_queue);
  checked_free(threads);
  checked_free(thread_ids);
  checked_free(process_table);
  pthread_mutex_destroy(&table_lock);
 
}

ProcessIdT simulator_create_process(EvaluatorCodeT const code) {
  ProcessIdT pid;
  
  //if no available ids then return -1
  if(blocking_queue_pop(pid_queue,&pid) != 0){
    return -1;
  }
  
  //save info about process
  pthread_mutex_lock(&table_lock);
  
  ProcessT* process = &process_table[pid - 1]; 
  process->pid = pid;
  process->eval_code = code;
  process->pc = 0; //pc always set to 0 when initialising process
  process->completed = 0;
  process->state = ready;
  
  pthread_mutex_unlock(&table_lock);
  
  //init the semaphore
  sem_init(&process->semaphore, 0, 0);
  
  //add initialised process id to ready queue
  blocking_queue_push(ready_queue, pid);
  
  formatted_logger(pid , "Created");
  
  return pid;
  
}

void simulator_wait(ProcessIdT pid) {
  //retrieve from pcb
  ProcessT* process = &process_table[pid - 1];
  
  // Log that we are waiting for the process
  formatted_logger(pid,"Waiting to finish");
  
  //wait for process to finish
  pthread_mutex_lock(&table_lock);
  
  if (process->state == terminated) {
    pthread_mutex_unlock(&table_lock);
    
    // directly clean up the terminated process without waiting
    sem_destroy(&process->semaphore);
    memset(process, 0, sizeof(ProcessT));
    return;
  }
  
  //post in the case ready queue is terminated
  if(ready_queue->terminated == 1)
  {
    sem_post(&process->semaphore);
  }
  
  pthread_mutex_unlock(&table_lock);
  
  //wait for process to terminate
  sem_wait(&process->semaphore);
  
  sem_destroy(&process->semaphore);
  
  //clear entry in process table
  memset(process, 0, sizeof(ProcessT));
  
  //reuse pid by adding back to pid queue
  blocking_queue_push(pid_queue, pid);
}

void simulator_kill(ProcessIdT pid) {

  ProcessT* process = &process_table[pid - 1];
  
  pthread_mutex_lock(&table_lock);
  
  if(process->state != terminated){ //check not already terminated
    //set state 
    process->state = terminated;
    formatted_logger(pid , "Killed");
  }
  
  pthread_mutex_unlock(&table_lock);
  
}

void* simulator_event(void *arg) {
  useconds_t interval = *(useconds_t *)arg;
  
  //check event source is not terminated
  while(!check_termination()){
    
    usleep(interval);
    ProcessIdT pid;
    
    //use non blocking queue as we're checking every interval
    int result = non_blocking_queue_pop(event_queue, &pid);
    
    //in the case queue is empty 
    if(result != 0){
      continue;
    }
    
    formatted_logger(pid, "Moved to ready queue");
    
    //move to ready queue to be evaluated
    blocking_queue_push(ready_queue,pid);
    
  }
  
  return NULL;
}

void formatted_logger(int id , const char* message)
{
  char log_msg[100];
  snprintf(log_msg, sizeof(log_msg), "Process ID: %i - %s", id,message);
  logger_write(log_msg);
}
