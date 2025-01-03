#ifndef _SIMULATOR_H_
#define _SIMULATOR_H_

#include "evaluator.h"
#include <stddef.h>
#include <pthread.h>
#include "blocking_queue.h"

typedef unsigned int ProcessIdT;

typedef enum ProcessState {
  unallocated,
  ready,
  running,
  blocked,
  terminated
} ProcessStateT;

typedef struct Process {
  int pid;
  EvaluatorCodeT eval_code; 
  unsigned int pc; //program counter
  int completed;  //flag to check if process is finished 
  sem_t semaphore; 
  ProcessStateT state;
}ProcessT;

void simulator_start(int threads, int max_processes);
void simulator_stop();

ProcessIdT simulator_create_process(EvaluatorCodeT const code);
void simulator_wait(ProcessIdT pid);
void simulator_kill(ProcessIdT pid);
void *simulator_event(void *arg);
void *simulator_routine(void *arg);
void print_evaluator_result(EvaluatorResultT result);

#endif
