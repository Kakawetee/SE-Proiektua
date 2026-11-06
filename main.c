#include "simulazioa.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

// Aldagai globalak simulaziorako
int ticks_erloju = 0;
int sim = 1;
int done = 0;
int tenp_kop = 2; // 2 tenporizadore

// Timer-aren mutexa eta kondizio-aldagaiak (sasikodeko mutex, cond eta cond2)
pthread_mutex_t mutex_timer = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_timer = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_timer2 = PTHREAD_COND_INITIALIZER;

// Mutex eta kondizio-aldagaia prozesuen ilara kudeatzeko
pthread_mutex_t mutex_ilara = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_ilara = PTHREAD_COND_INITIALIZER;

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
		init_machine(&m, atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
	}

	while (1)
	{
	}

	return 0;
}
