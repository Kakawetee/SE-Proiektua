#ifndef HARIAK_H
#define HARIAK_H

#include "simulazioa.h"

typedef struct {
    int clock_freq;
    int timer_freq;
    int duration;
    ProcessQueue *queue;
    Machine *machine;
    SchedPolicy policy;
} ThreadArgs;

ThreadArgs sortuThreadArgs(int clock_freq, int timer_freq, int duration, 
    ProcessQueue *queue, Machine *machine, SchedPolicy policy);
void *clockErrutina(void *arg);
void *timerErrutina(void *arg);
void *pgErrutina(void *arg);
void *schedulerErrutina(void *arg);

#endif