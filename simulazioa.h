#ifndef SIMULAZIOA_H
#define SIMULAZIOA_H

#include <pthread.h> // Necesario para el mutex

typedef struct
{
	int cpu_numb;
	int cores_per_cpu;
	int hwthreads_per_core;
} Machine;

typedef struct
{
	int pid;
	int life_time;
} PCB;

typedef struct Node
{
	PCB pcb;
	struct Node *next;
} Node;

typedef struct
{
	Node *head;
	Node *tail;
	int size;
	pthread_mutex_t mutex;
} ProcessQueue;

void init_machine(Machine *machine, int cpu_numb, int cores_per_cpu, int hwthreads_per_core);
void init_pcb(PCB *pcb, int pid, int life_time);
void init_node(Node *node, const PCB *pcb);
int init_process_queue(ProcessQueue *queue);
void destroy_process_queue(ProcessQueue *queue);
int create_process(ProcessQueue *queue, int pid, int life_time);
int dequeue_process(ProcessQueue *queue, PCB *pcb);

#endif
