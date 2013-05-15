#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#define BILLION  1000000000L;

struct timespec start, stop;



void startt();
void stopt();
double gettime();
