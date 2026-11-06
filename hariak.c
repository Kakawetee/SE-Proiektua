#include "simulazioa.h"
#include "hariak.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

extern int ticks_erloju;
extern int ticks_timer;
extern int sim;
extern pthread_mutex_t mutex_timer;
extern pthread_cond_t cond_timer;
extern pthread_mutex_t mutex_ilara;
extern pthread_cond_t cond_ilara;

ThreadArgs sortuThreadArgs(int clock_freq, int timer_freq, int duration, ProcessQueue *queue) {
    ThreadArgs args = {
        .clock_freq = clock_freq,
        .timer_freq = timer_freq,
        .duration = duration,
        .queue = queue
    };

    if (queue == NULL) {
        fprintf(stderr, "Error: Invalid arguments to sortuThreadArgs\n");
        return (ThreadArgs){0};
    }

    return args;
}

void *clockErrutina(void *arg) {
ThreadArgs *args = (ThreadArgs *)arg;
int itxoin = 1000000 / args->clock_freq; // Mikrosegundotan itxoin beharreko denbora

    while (ticks_erloju < args->duration) {
        usleep(itxoin); // Denbora pasa simulatu

        pthread_mutex_lock(&mutex_timer);
        ticks_erloju++;
        // Timerrei abisatu tick bat sortu dela
        pthread_cond_signal(&cond_timer);
        pthread_mutex_unlock(&mutex_timer);
    }

    // Bukaeran, simulazioa amaitu dela adierazi eta kondiziozko bariableak abisatu
    sim = 0;
    pthread_cond_broadcast(&cond_timer);
    pthread_cond_broadcast(&cond_ilara);

    return NULL;
}