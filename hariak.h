#ifndef HARIAK_H
#define HARIAK_H

#include "simulazioa.h"

typedef struct {
    int clock_freq;
    int timer_freq;
    int duration;
    ProcessQueue *queue;
} ThreadArgs;

ThreadArgs sortuThreadArgs(int clock_freq, int timer_freq, int duration, ProcessQueue *queue);
void *clockErrutina(void *arg);

#endif