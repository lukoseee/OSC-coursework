#include "blocking_queue.h"
#include "utilities.h"
#include <stdio.h>

void blocking_queue_terminate(BlockingQueueT* queue) {
  queue->terminated = 1;
  
  // Wake up all threads waiting on the semaphore
  while (sem_trywait(&queue->queue_sem) == 0) {
    sem_post(&queue->queue_sem);  // Unblock waiting threads
  }
  
}

void blocking_queue_create(BlockingQueueT* queue) {
  queue->front = queue->rear = NULL;
  
  //set to unterminated
  queue->terminated = 0;
  
  //init mutex and semaphore
  pthread_mutex_init(&queue->lock, NULL);
  sem_init(&queue->queue_sem,0,0);
}

void blocking_queue_destroy(BlockingQueueT* queue) {

  if (queue == NULL) return; // Guard against NULL queue

  pthread_mutex_lock(&queue->lock); // Lock the mutex

  ListT* current = queue->front;
  while (current != NULL) {
      ListT* next = current->succ;
      checked_free(current);
      current = next;
  }
  queue->front = queue->rear = NULL;

  pthread_mutex_unlock(&queue->lock); // Unlock the mutex
  pthread_mutex_destroy(&queue->lock); // Destroy the mutex
  
  //free semaphore
  sem_destroy(&queue->queue_sem); 
  
}

void blocking_queue_push(BlockingQueueT* queue, unsigned int value) {
  //lock mutex
  pthread_mutex_lock(&queue->lock);
  
  //safe memory alloc
  ListT* new_node = (ListT*)checked_malloc(sizeof(ListT));
  
  //set value and rear
  new_node->value = value;
  new_node->succ = NULL;
  new_node->pred = queue->rear;
  
  if(queue->rear == NULL){
    //if queue empty then front is new value
    queue->front = new_node;
  }else{
    //link current rear node to next node
    queue->rear->succ = new_node;
  }
  queue->rear = new_node;
  
  //unlock
  pthread_mutex_unlock(&queue->lock);
  
  //signal semaphore
  sem_post(&queue->queue_sem);
}

int blocking_queue_pop(BlockingQueueT* queue, unsigned int* value) {
  pthread_mutex_lock(&queue->lock);
  
  // If the queue is terminated and still empty, return failure
  if ( (queue->terminated == 1) && (blocking_queue_empty(queue)) ) {
      //unlock
      pthread_mutex_unlock(&queue->lock);
      return 1; // Failure
  }

   // If the queue is empty, block until an item is pushed
   while (blocking_queue_empty(queue)) {
    // Unlock the mutex before waiting on the semaphore
    pthread_mutex_unlock(&queue->lock);
    
    // Block and wait for a signal that an item is available in the queue
    sem_wait(&queue->queue_sem);  // Wait on the semaphore
    
    // Re-lock the mutex after being signaled
    pthread_mutex_lock(&queue->lock);
  }

  //store int in value
  *value = queue->front->value;
  //save pointer to previous front
  ListT* prevFront = queue->front;
  //set front to be the next node in queue
  queue->front = queue->front->succ;
  
  //if queue is now empty, reset rear aswell
  if(queue->front == NULL){
    queue->rear == NULL;
  }
  //free previous front
  checked_free(prevFront);
  
  //unlock
  pthread_mutex_unlock(&queue->lock);
  return 0;
}

int blocking_queue_empty(BlockingQueueT* queue) {
  return queue->front == NULL;
}

int blocking_queue_length(BlockingQueueT* queue) {
  int size = 0;
  ListT* current = queue->front;
  //searches until end of queue
  while(current != NULL){
    size++;
    current = current->succ;
  }
  return size;
}
