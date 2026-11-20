#include "simulazioa.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

// *** SORTUTAKO STRUCT-AK HASIERATZEKO FUNTZIOAK *** //
////////////////////////////////////////////////////////
int init_machine(Machine *m, int cpus, int cores, int hwthreads)
{
	if (m == NULL)
	{
		return EINVAL;
	}

	m->cpu_numb = cpus;
	m->cores_per_cpu = cores;
	m->hwthreads_per_core = hwthreads;
	m->total_hwthreads = cpus * cores * hwthreads;
	m->free_hwthreads = m->total_hwthreads;

	m->hwthreads = calloc(m->total_hwthreads, sizeof(HWThread));
	if(m->hwthreads == NULL) return ENOMEM;

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

	for(int i = 0; i<m->total_hwthreads; i++)
	{
		free(m->hwthreads[i].process);
	}
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

void init_node(Node *node, PCB *pcb)
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

int enqueue_pcb(ProcessQueue *queue, PCB *pcb)
{
	Node *node;

	if(queue == NULL || pcb == NULL) return EINVAL;

	node = malloc(sizeof(*node));
	if(node == NULL) return ENOMEM;
	init_node(node,pcb);

	if(pthread_mutex_lock(&queue->mutex) != 0)
	{
		free(node);
		return EAGAIN;
	}

	if(queue->tail == NULL) queue->head = node;
	else					queue->tail->next = node;
	queue->tail = node;
	queue->size++;

	pthread_mutex_unlock(&queue->mutex);
	return 0;
}

int create_process(ProcessQueue *queue, int pid, int life_time, int arrival_time)
{
	PCB *pcb;
	int result;

	if (queue == NULL) return EINVAL;

	pcb = malloc(sizeof(*pcb));
	if (pcb == NULL) return ENOMEM;
	
	init_pcb(pcb, pid, life_time, arrival_time); // Pcb-a direktu pasatzen zaie, kopia sortuz gero, &pcb sartu beharko litzateke

	result = enqueue_pcb(queue,pcb);
	if(result != 0) free(pcb);

	return result;

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

// FCFS: el PG añade al final de la cola, así que el primero en llegar está en head.
PCB *policy_fcfs(ProcessQueue *ready)
{
	PCB *pcb = NULL;

	if (dequeue_process(ready, &pcb) != 0) return NULL;
	
	return pcb;
}

PCB *policy_sjf(ProcessQueue *ready)
{
	Node *prev = NULL;
	Node *cur;
	Node *best = NULL;
	Node *best_prev = NULL;
	PCB *pcb;

	if(ready == NULL) return NULL;

	if(pthread_mutex_lock(&ready->mutex) != 0) return NULL;

	for (cur = ready->head; cur != NULL; prev = cur, cur = cur->next)
	{
		if(best == NULL || cur->pcb->life_time < best->pcb->life_time)
		{
			best = cur;
			best_prev = prev;
		}
	}

	if (best == NULL)
	{
		pthread_mutex_unlock(&ready->mutex);
		return NULL;
	}

	// best_prev head zan, beraz, head hurrengoa izango da
	if (best_prev == NULL) ready->head = best->next;
	// Bestela, jauzi egiten da
	else best_prev->next = best->next;

	if(best == ready->tail) ready->tail = best->prev;
	ready->size--;
	pcb = best->pcb;

	pthread_mutex_unlock(&ready->mutex);
	free(best);

	return pcb;


}

int machine_free_count(Machine *m) 
{
	
	int libre;
	pthread_mutex_lock(&m->mutex);
	libre = m->free_hwthreads;
	pthread_mutex_unlock(&m->mutex);

	return libre;
}

int dispatch_process(Machine *m, PCB *pcb, HWThread **hw_out)
{
	if (m == NULL || pcb == NULL) return EINVAL;

	pthread_mutex_lock(&m->mutex);
	for (int i = 0; i < m->total_hwthreads; i++)
	{
		if(!m->hwthreads[i].busy)
		{
			m->hwthreads[i].busy = 1;
			m->hwthreads[i].process = pcb;
			pcb->hwthread = &m->hwthreads[i];
			pcb->egoera = PROC_RUNNING;
			m->free_hwthreads--;
			// NULL baliozkoa da, use-after-free arazoa solbatzeko erabiltzen da,
			// schedulerrak prozesua zein harietan geratu den jakin behar du
			// pcb-a berriro irakurri behar gabe
			if (hw_out != NULL) *hw_out = &m->hwthreads[i];
			pthread_mutex_unlock(&m->mutex);
			return 0;
		}
	}
	pthread_mutex_unlock(&m->mutex);

	return ENOSPC;
}

void machine_consume_tick(Machine *m, ProcessQueue *finished)
{
	pthread_mutex_lock(&m->mutex);
	for (int i = 0; i < m->total_hwthreads; i++) {
		HWThread *hw = &m->hwthreads[i];
		PCB *pcb = hw->process;

		// Aske badago ez da zertan tratatu behar
		if(!hw->busy || pcb == NULL) continue;

		pcb->life_time--;
		if(pcb->life_time<=0)
		{
			printf("[Clock] Prozesua %d amaituta (CPU %d, core %d, thread %d)\n",
			       pcb->pid, hw->cpu_id, hw->core_id, hw->thread_id);

			pcb->egoera = PROC_FINISHED;
			pcb->hwthread = NULL;
			hw->process = NULL;
			hw->busy = 0;
			m->free_hwthreads++;

			if (enqueue_pcb(finished, pcb) != 0) free(pcb);
		}
	}
	pthread_mutex_unlock(&m->mutex);
}
