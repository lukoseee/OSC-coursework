#include "non_blocking_queue.h"
#include "utilities.h"

#include <assert.h>

void non_blocking_queue_create(NonBlockingQueueT* queue) {
  queue->front = queue->rear = NULL;
  //init mutex
  pthread_mutex_init(&queue->lock, NULL);
}

void non_blocking_queue_destroy(NonBlockingQueueT* queue) {
  //lock mutex
  pthread_mutex_lock(&queue->lock);
  
  ListT* current = queue->front;
  while ( current != NULL )
  { 
    //free and set current to next;
    ListT* next = current->succ;
    checked_free(current);
    current = next;
  }
  queue->front = queue->rear = NULL;
  
  //unlock and destroy mutex
  pthread_mutex_unlock(&queue->lock);
  pthread_mutex_destroy(&queue->lock);
  
}

void non_blocking_queue_push(NonBlockingQueueT* queue, unsigned int value) {
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
}

int non_blocking_queue_pop(NonBlockingQueueT* queue, unsigned int* value) {
  pthread_mutex_lock(&queue->lock);
  
  if( non_blocking_queue_empty(queue) ){
    //unlock
    pthread_mutex_unlock(&queue->lock);
    return 1;
  }
  
  //store int in value
  *value = queue->front->value;
  //save pointer to previous front
  ListT* prevFront = queue->front;
  //set front to be the next node in queue
  queue->front = queue->front->succ;
  
  //if queue is now empty, reset rear aswell
  if(queue->front == NULL){
    queue->rear = NULL;
  }
  //free previous front
  checked_free(prevFront);
  
  //unlock
  pthread_mutex_unlock(&queue->lock);
  return 0;
}

int non_blocking_queue_empty(NonBlockingQueueT* queue) {
  return queue->front == NULL;
}

int non_blocking_queue_length(NonBlockingQueueT* queue) {
  int size = 0;
  ListT* current = queue->front;
  //searches until end of queue
  while(current != NULL){
    size++;
    current = current->succ;
  }
  return size;
}
