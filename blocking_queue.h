#ifndef _BLOCKING_QUEUE_H_
#define _BLOCKING_QUEUE_H_

#include "list.h"
#include <pthread.h>
#include <semaphore.h>

typedef struct BlockingQueue {
  ListT* front;
  ListT* rear;
  pthread_mutex_t lock;
  int terminated;
  sem_t queue_sem;
} BlockingQueueT;

void blocking_queue_create(BlockingQueueT* queue);
void blocking_queue_destroy(BlockingQueueT* queue);

void blocking_queue_push(BlockingQueueT* queue, unsigned int value);
int blocking_queue_pop(BlockingQueueT* queue, unsigned int* value);

int blocking_queue_empty(BlockingQueueT* queue);
int blocking_queue_length(BlockingQueueT* queue);

void blocking_queue_terminate(BlockingQueueT* queue);

#endif
