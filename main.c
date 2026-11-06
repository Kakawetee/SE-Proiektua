#include "simulazioa.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

//Aldagai globalak simulaziorako
int ticks_erloju = 0;
int ticks_timer = 0;
int sim = 1;

int cpus = 2;
int cores = 4;
int hwthreads = 2;
int clock_freq = 1;
int timer_freq = 5;
int duration = 100;

//Mutex eta kondiziozko bariableak timerentzak
pthread_mutex_t mutex_timer = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_timer = PTHREAD_COND_INITIALIZER;
//Mutex eta kondiziozko bariableak prozesuen ilara kudeatzeko
pthread_mutex_t mutex_ilara = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_ilara = PTHREAD_COND_INITIALIZER;

int main(int argc, char *argv[])
{
	
	for(int i=0; i<argc; i+=2)
	{
		if(!isdigit(argv[i+1])){
			printf("Emandako parametroa ez da zenbakizkoa: %c, exekuzioa etengo da.",argv[i+1]);
			return 1;
		}
		switch (argv[i]) {
			case '-cpus':
				cpus = argv[i+1];
				break;
			case '-cores':
				cores = argv[i+1];
				break;
			case '-hwthreads':
				hwthreads = argv[i+1];
				break;
			case '-clock-freq':
				clock_freq = argv[i+1];
				break;
			case '-timer-freq':
				timer_freq = argv[i+1];
				break;
			case '-duration':
				duration = argv[i+1];
				break;
			default:
				printf("Emandako parametroa ez da baliozkoa: %c\n",argv[i]);
				break;
	}


	while (1)
	{
		
	}

	return 0;
}
