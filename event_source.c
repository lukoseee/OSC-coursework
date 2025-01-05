#include "event_source.h"
#include "utilities.h"
#include "simulator.h"

int terminate; //global variable to keep track of whether event source has ended 
static pthread_t event_thread; //threads
static pthread_mutex_t event_lock;

void event_source_start(useconds_t interval) {
  terminate = 0;
  
  //pass interval as argument to simulator_event
  pthread_create(&event_thread, NULL, simulator_event, &interval);
  
}

void event_source_stop() {

  terminate = 1;
  
  pthread_join(event_thread, NULL);
  
}

int check_termination(){
  return terminate;
}

