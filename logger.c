#include "logger.h"
#include "utilities.h"

int id;

void logger_start() {
  fprintf(stdout , "Logger Started\n");
}

void logger_stop() {
  fprintf(stdout , "Logger Closed\n");
  id = 0;
}

void logger_write(char const* message) {
  time_t t = time(NULL);
  struct tm date;
  
  //error checking
  if ( gmtime_r(&t, &date) == NULL ) {
    fprintf(stderr , "Error with gmtime_r");
  }
  
  //printing to standard out in specified format
  fprintf(stdout, "%i : %02d:%02d:%02d : %s\n", id, date.tm_hour , date.tm_min , date.tm_sec, message);
  id++;
}
