#include "non_blocking_queue.h"
#include "utilities.h"
#include <stdio.h>
#include <assert.h>


NonBlockingQueueT* setup()
{ 
  //setup queue for each test 
  NonBlockingQueueT* queue = (NonBlockingQueueT*)checked_malloc( sizeof(NonBlockingQueueT) );
  non_blocking_queue_create(queue);
  return queue;
}

void teardown(NonBlockingQueueT* queue){
  //free queue after each test 
  non_blocking_queue_destroy(queue);
  free(queue);
}

void test_empty_creation() {
  printf("testing empty creation/destruction of non-blocking queues\n");
  
  NonBlockingQueueT* queue = setup();
  assert(non_blocking_queue_empty(queue));
  assert(non_blocking_queue_length(queue) == 0);
  teardown(queue);
}


void test_push(){
  printf("testing push\n");
  
  //alloc queue
  NonBlockingQueueT* queue = setup();
  
  //push values
  non_blocking_queue_push(queue, 1);
  non_blocking_queue_push(queue, 2);
  non_blocking_queue_push(queue, 3);
  
  //check front and rear
  assert(queue->front->value == 1);
  assert(queue->rear->value == 3);
  
  //check length
  assert(non_blocking_queue_length(queue) == 3);
  
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
  NonBlockingQueueT* queue = setup();
  
  //check empty queue
  unsigned int value = 0; 
  assert(non_blocking_queue_pop(queue,&value) == 1);
  
  //clean up
  teardown(queue);
  
}

void test_pop(){
  printf("testing pop\n");
  
  //alloc queue
  NonBlockingQueueT* queue = setup();
  unsigned int value = 0;
  
  //push values
  non_blocking_queue_push(queue, 1);
  non_blocking_queue_push(queue, 2);
  non_blocking_queue_push(queue, 3);
  
  //pop 1
  assert(non_blocking_queue_pop(queue,&value) == 0);
  
  //check value has been updated
  assert(value == 1);
  
  //check front is updated
  assert(queue->front->value == 2);
  
  //check length
  assert(non_blocking_queue_length(queue) == 2);
  
  teardown(queue);
  
}

int main() {
  test_empty_creation();
  test_push();
  test_pop_failure();
  test_pop();
  return 0;
}
