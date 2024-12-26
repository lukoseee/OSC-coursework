#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <stdio.h>
#include <time.h>

void logger_start();
void logger_stop();
void logger_write(char const* message);

#endif
