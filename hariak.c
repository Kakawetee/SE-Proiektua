#include "simulazioa.h"
#include "hariak.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

extern int ticks_erloju;
extern int sim;
extern int done;
extern int tenp_kop;

extern pthread_mutex_t mutex_timer;
extern pthread_cond_t cond_timer;
extern pthread_cond_t cond_timer2;
extern pthread_cond_t cond_pg;

extern pthread_mutex_t mutex_ilara;
extern pthread_cond_t cond_ilara;

ThreadArgs sortuThreadArgs(int clock_freq, int timer_freq, int duration, ProcessQueue *queue) {
    // Hari guztiek behar duten konfigurazioa egitura bakarrean biltzen du.
    ThreadArgs args = {
        .clock_freq = clock_freq,
        .timer_freq = timer_freq,
        .duration = duration,
        .queue = queue
    };

    if (queue == NULL) {
        // Ilara nulua bada, konfigurazioa ez da balioduna.
        fprintf(stderr, "Error: Invalid arguments to sortuThreadArgs\n");
        return (ThreadArgs){0};
    }

    return args;
}

void *clockErrutina(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    if (args == NULL || args->clock_freq <= 0 || args->duration < 0) {
        // Argumentuak baliozkoak ez badira, haria ezin da exekutatu.
        fprintf(stderr, "Error: Invalid arguments to clockErrutina\n");
        return NULL;
    }
    // usleep() erabiliko dugunez mikrosegunduekin egingo dugu lan
    // beraz, 1000000 mikrosegundu / frekuentzia = periodoa
    int itxoin = 1000000 / args->clock_freq;

    // Erlojuak tick bat sortzen du konfiguratutako maiztasunarekin.
    while (ticks_erloju < args->duration) {
        // Hurrengo tick-a iritsi arte itxaroten du.
        usleep(itxoin);

        pthread_mutex_lock(&mutex_timer);
        // Tenporizadore guztiak prest egon arte itxaroten du.
        while (done < tenp_kop) {
            pthread_cond_wait(&cond_timer, &mutex_timer);
        }
        // Hurrengo ziklorako prest dauden tenporizadoreen kontagailua berrezartzen du.
        done = 0;
        ticks_erloju++;
        // Tenporizadore guztiak batera esnatzen ditu tick berria prozesatzeko.
        pthread_cond_broadcast(&cond_timer2);
        // PGri tick berria jakinarazten dio; PGk bere hurrengo tartea aukeratzen du.
        pthread_cond_signal(&cond_pg);
        pthread_mutex_unlock(&mutex_timer);
    }

    // Azken tick-a amaitu arte itxaron aurretik simulazioa ez du amaitzen.
    pthread_mutex_lock(&mutex_timer);
    while (done < tenp_kop) {
        // mutex_timer askatu timerrak sartzeko, haria lo utzi, 
        // eta pthread_cond_signal() egiten da done++ ostean, tenp
        pthread_cond_wait(&cond_timer, &mutex_timer);
    }
    // Simulazioaren amaiera adierazten du eta tenporizadoreak esnatzen ditu.
    sim = 0;

    // Timerrari tick berria baimentzen dio
    pthread_cond_broadcast(&cond_timer2);
    pthread_cond_signal(&cond_pg);
    pthread_mutex_unlock(&mutex_timer);

    pthread_mutex_lock(&mutex_ilara);
    // Ilaran itxaroten dauden hariak ere esnatzen ditu.
    pthread_cond_broadcast(&cond_ilara);
    
    pthread_mutex_unlock(&mutex_ilara);

    return NULL;
}

void *timerErrutina(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    if (args == NULL || args->timer_freq < 1) {
        // Maiztasun baliodunik gabe ezin da timer-aren lana egin.
        fprintf(stderr, "Error: Invalid arguments to timerErrutina\n");
        return NULL;
    }

    int nire_ticks = 0;

    // Mutex-a begiztaren aurretik hartzen du.
    pthread_mutex_lock(&mutex_timer);
    while (sim) {
        // Timer hau prest dagoela jakinarazten dio erlojuari.
        done++;
        pthread_cond_signal(&cond_timer);
        // Erlojuak tick berria baimendu arte itxaroten du.
        pthread_cond_wait(&cond_timer2, &mutex_timer);

        if (!sim) {
            break;
        }

        // Timer honek prozesatutako tick kopurua handitzen du.
        nire_ticks++;
        pthread_mutex_unlock(&mutex_timer);

        // Ezarritako maiztasunera iristean, PG eta scheduler-a esnatzen ditu.
        if (nire_ticks % args->timer_freq == 0) {
            pthread_mutex_lock(&mutex_ilara);
            pthread_cond_broadcast(&cond_ilara);
            pthread_mutex_unlock(&mutex_ilara);
        }

        pthread_mutex_lock(&mutex_timer);
    }

    pthread_mutex_unlock(&mutex_timer);
    return NULL;
}

void *pgErrutina(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    ProcessQueue *queue;
    int pid = 1;
    int azken_tick = 0;
    int hurrengo_sorrera = 1;
    int tick_kop = 0;

    if (args == NULL || args->queue == NULL || args->timer_freq < 1) {
        // Ilara gabe ezin da prozesurik sortu.
        fprintf(stderr, "Error: Invalid queue in pgErrutina\n");
        return NULL;
    }
    queue = args->queue;

    while (sim) {
        pthread_mutex_lock(&mutex_timer);
        // Baldintzak tick-ak ez galtzea eta irakurketa partekatua babestea bermatzen du.
        while (sim && ticks_erloju == azken_tick) {
            pthread_cond_wait(&cond_pg, &mutex_timer);
        }

        if (!sim) {
            pthread_mutex_unlock(&mutex_timer);
            break;
        }
        azken_tick = ticks_erloju;
        pthread_mutex_unlock(&mutex_timer);

        tick_kop++;
        // Oraindik ez da aukeratutako ausazko tartea bete.
        if (tick_kop < hurrengo_sorrera) {
            continue;
        }

        // Prozesu bat ausazko iraupenarekin sortu eta ilaran sartzen du.
        int life_time = (rand() % 10) + 1;
        if (create_process(queue, pid, life_time, azken_tick) == 0) {
            printf("[PG] Prozesua sortuta, PIDa: %d\n", pid);
            pid++;
            // Scheduler-ari ilaran prozesu berria dagoela jakinarazten dio.
            pthread_mutex_lock(&mutex_ilara);
            pthread_cond_signal(&cond_ilara);
            pthread_mutex_unlock(&mutex_ilara);
        }
        tick_kop = 0;
        // Hurrengo prozesua beste tarte aldakor baten ondoren sortuko da.
        hurrengo_sorrera = (rand() % args->timer_freq) + 1;

    }

    return NULL;
}

void *schedulerErrutina(void *arg) {
    ProcessQueue *queue = (ProcessQueue *)arg;
    PCB *process = NULL;

    if (queue == NULL) {
        // Ilara gabe scheduler-ak ezin du prozesurik kudeatu.
        fprintf(stderr, "Error: Invalid queue in schedulerErrutina\n");
        return NULL;
    }

    while (sim) {
        pthread_mutex_lock(&mutex_ilara);
        // Ilara batean prozesu bat dagoela jakinarazi arte itxaroten du.
        pthread_cond_wait(&cond_ilara, &mutex_ilara);

        if (!sim) {
            pthread_mutex_unlock(&mutex_ilara);
            break;
        }
        pthread_mutex_unlock(&mutex_ilara);

        // Ilaratik prozesu bat ateratzen saiatzen da.
        if (dequeue_process(queue, &process) == 0) {
            printf("[Scheduler] Prozesua bidaltzen, PIDa: %d\n", process->pid);
            free(process) // Tenporala, dispatcherra oraindik ez dugulako programatu
        } else {
            printf("[Scheduler] Ez dago prozesurik ilaran.\n");
        }
    }

    return NULL;
}