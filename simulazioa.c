#include "simulazioa.h"

#include <errno.h>
#include <stdlib.h>

// *** SORTUTAKO STRUCT-AK HASIERATZEKO FUNTZIOAK *** //
////////////////////////////////////////////////////////
void init_machine(Machine *m, int cpus, int cores, int hwthreads)
{
	if (m == NULL)
	{
		return;
	}

	m->cpu_numb = cpus;
	m->cores_per_cpu = cores;
	m->hwthreads_per_core = hwthreads;
	m->total_hwthreads = cpus * cores * hwthreads;
	m->free_hwthreads = m->total_hwthreads;

	m->hwthreads = calloc(m->total_hwthreads, sizeof(HWThread));
	if(m->hwthreads == NULL) reutrn ENOMEM;

	for (int i = 0; i < m->total_hwthreads; i++) {
        m->hwthreads[i].cpu_id    = i / (cores * hwthreads);
        m->hwthreads[i].core_id   = (i / hwthreads) % cores;
        m->hwthreads[i].thread_id = i % hwthreads;
        m->hwthreads[i].busy = 0;
        m->hwthreads[i].process = NULL;
    }
    return pthread_mutex_init(&m->mutex, NULL);
}

void destroy_machine(Machine *m)
{
	if(m == NULL) return;
	free(m->hwthreads);
	m->hwthreads = NULL;
	pthread_mutex_destroy(&m->mutex);
}

void init_pcb(PCB *pcb, int pid, int life_time, int arrival_time)
{
	if (pcb == NULL) return;

	pcb->pid = pid;
	pcb->egoera = PROC_READY;
	pcb->life_time = life_time;
	pcb->arrival_time = arrival_time;
	pcb->prio = 0;
	pcb->quantum = 0;
	pcb->hwthread = NULL;
}

void init_node(Node *node, const PCB *pcb)
{
	if (node == NULL || pcb == NULL)
	{
		return;
	}

	node->pcb = pcb; // Nodoak PCB-ari egiten dio erreferentzia zuzenean, kopia egitea aurrezteko
	node->next = NULL;
}

int init_process_queue(ProcessQueue *queue)
{
	if (queue == NULL)
	{
		return EINVAL;
	}

	queue->head = NULL;
	queue->tail = NULL;
	queue->size = 0;

	return pthread_mutex_init(&queue->mutex, NULL);
}
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

// Ilara deusestatzeko erabiltzen den funtzioa, Pointerrak = NULL
void destroy_process_queue(ProcessQueue *queue)
{
	Node *node;
	Node *next;

	if (queue == NULL)
	{
		return;
	}

	node = queue->head;
	while (node != NULL)
	{
		next = node->next;
		free(node->pcb); // Ilaran dauden PCB-ak hemen askatzen dira zuzenean
		free(node);
		node = next;
	}

	queue->head = NULL;
	queue->tail = NULL;
	queue->size = 0;
	pthread_mutex_destroy(&queue->mutex);
}

//
int create_process(ProcessQueue *queue, int pid, int life_time, int arrival_time)
{
	Node *node;
	PCB *pcb;
	int result;

	if (queue == NULL)
	{
		return EINVAL;
	}

	pcb = malloc(sizeof(*pcb));
	if (pcb == NULL)
	{
		return ENOMEM;
	}
	
	node = malloc(sizeof(*node));
	if (node == NULL)
	{
		return ENOMEM;
	}

	init_pcb(pcb, pid, life_time, arrival_time); // Pcb-a direktu pasatzen zaie, kopia sortuz gero, &pcb sartu beharko litzateke
	init_node(node, pcb);

	// main.c-n begiratu 0 ez badu bueltatzen akatsen bat egon da, beraz berriro
	// prozesua sortzen saiatu beharko litzateke
	result = pthread_mutex_lock(&queue->mutex);
	if (result != 0)
	{
		free(pcb)
		free(node);
		return result;
	}

	// Hutsa badagao, head-era atzitu
	if (queue->tail == NULL)
	{
		queue->head = node;
		// Kontrakoan, bukaeran gehitu
	}
	else
	{
		queue->tail->next = node;
	}

	queue->tail = node;
	queue->size++;

	// lock egitean egin beharreko berdina, main.c-n akatsekin adi
	result = pthread_mutex_unlock(&queue->mutex);
	if (result != 0)
	{
		return result;
	}

	return 0;
}

// Ilaratik prozesua ateratzeko
int dequeue_process(ProcessQueue *queue, PCB **pcb)
{
	Node *node;
	int result;

	if (queue == NULL || pcb == NULL)
	{
		return EINVAL;
	}

	result = pthread_mutex_lock(&queue->mutex);
	if (result != 0)
	{
		return result;
	}

	// Ilara hutsa badago, mutex askatu eta itzuli errore kodea
	node = queue->head;
	if (node == NULL)
	{
		pthread_mutex_unlock(&queue->mutex);
		return ENOENT;
	}

	// Ilarako elementu bakarra borratu behar den prozesua bada
	queue->head = node->next;
	if (queue->head == NULL)
	{
		queue->tail = NULL;
	}
	queue->size--;
	*pcb = node->pcb;

	result = pthread_mutex_unlock(&queue->mutex);
	free(node);

	return result;
}
