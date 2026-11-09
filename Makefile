CC = gcc
CFLAGS = -Wall -pthread

all: simulatzailea

simulatzailea: main.o simulazioa.o hariak.o
	$(CC) $(CFLAGS) -o simulatzailea main.o simulazioa.o hariak.o

main.o: main.c simulazioa.h hariak.h
	$(CC) $(CFLAGS) -c main.c

simulazioa.o: simulazioa.c simulazioa.h
	$(CC) $(CFLAGS) -c simulazioa.c

hariak.o: hariak.c hariak.h simulazioa.h
	$(CC) $(CFLAGS) -c hariak.c

clean:
	rm -f *.o simulatzailea