#include "simulazioa.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

//Aldagai globalak simulaziorako
int ticks_erloju = 0;
int ticks_timer = 0;
int sim = 1;
//Mutex eta kondiziozko bariableak timerentzak
int pthread_mutex_t mutex_timer = PTHREAD_MUTEX_INITIALIZER;
int pthread_cond_t cond_timer = PTHREAD_COND_INITIALIZER;
//Mutex eta kondiziozko bariableak prozesuen ilara kudeatzeko
int pthread_mutex_t mutex_ilara = PTHREAD_MUTEX_INITIALIZER;
int pthread_cond_t cond_ilara = PTHREAD_COND_INITIALIZER;

int main(int argc, char *argv[])
{

	if (argc != 4)
	{
		fprintf(stderr, "Parametro kopurua ez da zuzena");
		return 1;
	}
	else
	{
		Machine m;
		init_machine(&m, argv[1], argv[2], argv[3]);
	}

	while (1)
	{
	}

	return 0;
}
