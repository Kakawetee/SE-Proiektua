#include "simulazioa.h"

#include <errno.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{

	if (argc != 4)
	{
		print("Parametro kopurua ez da zuzena");
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
