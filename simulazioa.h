#ifndef SIMULAZIOA_H
#define SIMULAZIOA_H

#include <pthread.h> // Necesario para el mutex

typedef enum { PROC_READY, PROC_RUNNING, PROC_FINISHED } ProcessState;

typedef struct HWThread HWThread;
typedef struct PCB PCB;
typedef struct Node Node;

typedef struct
{
	int pid;
    ProcessState egoera;
	int life_time;
	int arrival_time;
    int prio; // Oraindik ez dugu inplementatuko
    int quantum; // Oraindik ez dugu inplementatuko
	HWThread *hwthread; // NULL ez badago exekuzioan
} PCB;

typedef struct
{
	int cpu_id, core_id, thread_id; // Pijada mesopotamica
    int busy;
    PCB *process;
} HWThread;

typedef struct
{
	int cpu_numb;
	int cores_per_cpu;
	int hwthreads_per_core;
	int total_hwthreads;
	int free_hwthreads;
    HWThread *hwthreads; // Mal definido, queria hacer una lista
	pthread_mutex_t mutex;
} Machine;


typedef struct
{
	PCB *pcb;
	Node *next;
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
