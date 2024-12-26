#include "blocking_queue.h"
#include "utilities.h"

#include <assert.h>
#include <stdio.h>
#include<unistd.h>

BlockingQueueT* setup()
{ 
  //setup queue for each test 
  BlockingQueueT* queue = (BlockingQueueT*)checked_malloc( sizeof(BlockingQueueT) );
  blocking_queue_create(queue);
  return queue;
}

void teardown(BlockingQueueT* queue){
  //free queue after each test 
  blocking_queue_destroy(queue);
  free(queue);
}

void test_empty_creation() {
  printf("testing empty creation/destruction of non-blocking queues\n");
  
  BlockingQueueT* queue = setup();
  assert(blocking_queue_empty(queue));
  assert(blocking_queue_length(queue) == 0);
  teardown(queue);
}


void test_push(){
  printf("testing push\n");
  
  //alloc queue
  BlockingQueueT* queue = setup();
  
  //push values
  blocking_queue_push(queue, 1);
  blocking_queue_push(queue, 2);
  blocking_queue_push(queue, 3);
  
  //check front and rear
  assert(queue->front->value == 1);
  assert(queue->rear->value == 3);
  
  //check length
  assert(blocking_queue_length(queue) == 3);
  
  //check intermediate values
  ListT* current = queue->front;
  assert(current->value == 1);
  current = current->succ;
  assert(current->value == 2);
  current = current->succ;
  assert(current->value == 3);
  
  //clean up
  teardown(queue);
}

void test_pop_failure(){
  printf("Testing empty queue\n");
  
  //alloc queue
  BlockingQueueT* queue = setup();
  
  unsigned int value = 0;
  
  //terminate queue
  blocking_queue_terminate(queue);
  
  //check if terminated 
  assert(queue->terminated == 1);
  
  //check pop fails
  assert(blocking_queue_pop(queue,&value) == 1);
  
  //clean up
  teardown(queue);
  
}



//alloc global queue for consumer thread
BlockingQueueT* global_queue;

void* consumer_routine(void* arg) {
  unsigned int value = 0;
  
  // pop from queue to block
  int result = blocking_queue_pop(global_queue, &value);

  // verify 
  assert(result == 0);
  assert(value == 42);
  
  //finish thread
  return NULL;
}

void test_blocking_behavior() {
  printf("Testing blocking behavior\n");
  
  global_queue = setup();
  
  printf("terminated == %i\n" , global_queue->terminated);
  
  //sample thread
  pthread_t consumer_thread;

  // start the consumer thread that will block on an empty queue
  pthread_create(&consumer_thread, NULL, consumer_routine, NULL); //pass queue to routine

  // sleep to ensure the consumer thread starts and blocks
  sleep(1);

  blocking_queue_push(global_queue, 42);

  // Wait for the consumer thread to finish
  pthread_join(consumer_thread, NULL);

  // Clean up
  teardown(global_queue);
}


void test_pop(){
  printf("testing pop\n");
  
  //alloc queue
  BlockingQueueT* queue = setup();
  unsigned int value = 0;
  
  //push values
  blocking_queue_push(queue, 1);
  blocking_queue_push(queue, 2);
  blocking_queue_push(queue, 3);
  
  //pop 1
  assert(blocking_queue_pop(queue,&value) == 0);
  
  //check value has been updated
  assert(value == 1);
  
  //check front is updated
  assert(queue->front->value == 2);
  
  //check length
  assert(blocking_queue_length(queue) == 2);
  
  teardown(queue);
  
}

int main() {
  test_empty_creation();
  test_push();
  test_pop_failure();
  test_pop();
  test_blocking_behavior();
  return 0;
}
