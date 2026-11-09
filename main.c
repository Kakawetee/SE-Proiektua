#include "simulazioa.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int cpus = 2;
int cores = 4;
int hwthreads = 2;
int clock_freq = 1;
int timer_freq = 5;
int duration = 100;

static int irakurriZenbakia(const char *parametroa, const char *balioa)
{
	char *amaiera;
	long zenbakia;

	errno = 0;
	zenbakia = strtol(balioa, &amaiera, 10); //strtol amaiera aldagaia irakurri ezin izan duen lehenengo karakterea gordetzen du.
	if (balioa == amaiera || *amaiera != '\0' || errno == ERANGE ||
		zenbakia < 1 || zenbakia > INT_MAX)
	{
		fprintf(stderr, "Balio numeriko baliogabea %s parametroarentzat: %s\n",
			parametroa, balioa);
		return -1;
	}

	return (int)zenbakia;
}

int main(int argc, char *argv[])
{
	for (int i = 1; i < argc; i += 2)
	{
		if (i + 1 >= argc)
		{
			fprintf(stderr, "Parametroak izen eta balio bikoteetan eman behar dira.\n");
			return 1;
		}

		int balioa;
		if (strcmp(argv[i], "-cpus") == 0)
		{
			balioa = irakurriZenbakia(argv[i], argv[i + 1]);
			cpus = balioa;
		}
		else if (strcmp(argv[i], "-cores") == 0)
		{
			balioa = irakurriZenbakia(argv[i], argv[i + 1]);
			cores = balioa;
		}
		else if (strcmp(argv[i], "-hwthreads") == 0)
		{
			balioa = irakurriZenbakia(argv[i], argv[i + 1]);
			hwthreads = balioa;
		}
		else if (strcmp(argv[i], "-clock-freq") == 0)
		{
			balioa = irakurriZenbakia(argv[i], argv[i + 1]);
			clock_freq = balioa;
		}
		else if (strcmp(argv[i], "-timer-freq") == 0)
		{
			balioa = irakurriZenbakia(argv[i], argv[i + 1]);
			timer_freq = balioa;
		}
		else if (strcmp(argv[i], "-duration") == 0)
		{
			balioa = irakurriZenbakia(argv[i], argv[i + 1]);
			duration = balioa;
		}
		else
		{
			fprintf(stderr, "Parametro ezezaguna: %s\n", argv[i]);
			return 1;
		}

		if (balioa < 0)
			return 1;
	}
	
	while (1)
	{
	}

	return 0;
}
