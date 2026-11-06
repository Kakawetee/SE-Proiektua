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
    // Hari errutinek behar duten konfigurazioa biltzen du.
    ThreadArgs args = {
        .clock_freq = clock_freq,
        .timer_freq = timer_freq,
        .duration = duration,
        .queue = queue
    };

    if (queue == NULL) {
        // Ilara nulua bada, ezin da konfigurazio baliodunik sortu.
        fprintf(stderr, "Error: Invalid arguments to sortuThreadArgs\n");
        return (ThreadArgs){0};
    }

    return args;
}

void *clockErrutina(void *arg) {
ThreadArgs *args = (ThreadArgs *)arg;
    if (args == NULL || args->clock_freq <= 0) {
        // Maiztasun baliodunik gabe ezin dira etenak sortu.
        fprintf(stderr, "Error: Invalid arguments to clockErrutina\n");
        return NULL;
    }
int itxoin = 1000000 / args->clock_freq; // Itxoin beharreko denbora

    // Erlojuak tick bat aurreratzen du kalkulatutako denbora-tarte bakoitzean.
    while (ticks_erloju < args->duration) {
        usleep(itxoin); // Denbora pasa simulatu

        pthread_mutex_lock(&mutex_timer);
        ticks_erloju++;
        // Timer-ari tick berri bat igaro dela jakinaraztentimer_freq zaio.
        pthread_cond_signal(&cond_timer);
        pthread_mutex_unlock(&mutex_timer);
    }

    // Erlojua amaitzean, simulazioa bukatu dela jakinarazi
    sim = 0;
    pthread_cond_broadcast(&cond_timer);
    pthread_cond_broadcast(&cond_ilara);

    return NULL;
}


void *timerErrutina(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;

    if (args == NULL || args->timer_freq <= 0) {
        // Maiztasun baliodunik gabe ezin dira etenak sortu.
        fprintf(stderr, "Error: Invalid arguments to timerErrutina\n");
        return NULL;
    }

    // Timer-ak aktibo jarraitzen du simulazioa martxan dagoen bitartean.
    while (sim) {
        pthread_mutex_lock(&mutex_timer);
        // Erlojuak beste tick bat igaro dela adierazi arte itxaroten du.
        pthread_cond_wait(&cond_timer, &mutex_timer);

        if (!sim) {
            // Erlojua amaitu da eta ez dago prozesatzeko tick gehiagorik.
            pthread_mutex_unlock(&mutex_timer);
            break;
        }

        // Timer-ak jasotako tick-ak zenbatzen ditu.
        ticks_timer++;
        pthread_mutex_unlock(&mutex_timer);

        // timer_freq tick bakoitzeko, ilarako hariei jakinarazten zaie.
        if (ticks_timer % args->timer_freq == 0) {
            pthread_cond_broadcast(&cond_ilara);
        }
    }

    return NULL;
}

void *pgErrutina(void *arg) {
    ProcessQueue *queue = (ProcessQueue *)arg;
    int pid = 1;

    if (queue == NULL) {
        fprintf(stderr, "Error: Invalid queue in pgErrutina\n");
        return NULL;
    }

    while (sim) {
        pthread_mutex_lock(&mutex_ilara);
        pthread_cond_wait(&cond_ilara, &mutex_ilara);

        if (!sim) {
            pthread_mutex_unlock(&mutex_ilara);
            break;
        }

        // Prozesu berri bat sortu eta ilaran sartzen du.
        int life_time = (rand() % 10) + 1;
        if (create_process(queue, pid, life_time) == 0) {
            printf("[PG] Prozesua sortuta, PIDa: %d\n", pid);
            pid++;
        }

        pthread_mutex_unlock(&mutex_ilara);
    }

    return NULL;
}

void *schedulerErrutina(void *arg) {
    ProcessQueue *queue = (ProcessQueue *)arg;
    PCB process;

    if (queue == NULL) {
        fprintf(stderr, "Error: Invalid queue in schedulerErrutina\n");
        return NULL;
    }

    while (sim) {
        pthread_mutex_lock(&mutex_ilara);
        pthread_cond_wait(&cond_ilara, &mutex_ilara);

        if (!sim) {
            pthread_mutex_unlock(&mutex_ilara);
            break;
        }

        // Ilaratik prozesu bat ateratzen saiatzen da.
        if (dequeue_process(queue, &process) == 0) {
            printf("[Scheduler] Prozesua bidaltzen, PIDa: %d\n", process.pid);
        }

        pthread_mutex_unlock(&mutex_ilara);
    }

    return NULL;
}