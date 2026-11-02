#include "simulazioa.h"

#include <errno.h>
#include <stdlib.h>

// *** SORTUTAKO STRUCT-AK HASIERATZEKO FUNTZIOAK *** //
////////////////////////////////////////////////////////
void init_machine(Machine* machine, int cpu_numb, int cores_per_cpu,
                  int hwthreads_per_core)
{
    if (machine == NULL) {
        return;
    }

    machine->cpu_numb = cpu_numb;
    machine->cores_per_cpu = cores_per_cpu;
    machine->hwthreads_per_core = hwthreads_per_core;
}

void init_pcb(PCB* pcb, int pid, int life_time)
{
    if (pcb == NULL) {
        return;
    }

    pcb->pid = pid;
    pcb->life_time = life_time;
}

void init_node(Node* node, const PCB* pcb)
{
    if (node == NULL || pcb == NULL) {
        return;
    }

    node->pcb = *pcb;
    node->next = NULL;
}

int init_process_queue(ProcessQueue* queue)
{
    if (queue == NULL) {
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
void destroy_process_queue(ProcessQueue* queue)
{
    Node* node;
    Node* next;

    if (queue == NULL) {
        return;
    }

    node = queue->head;
    while (node != NULL) {
        next = node->next;
        free(node);
        node = next;
    }

    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
    pthread_mutex_destroy(&queue->mutex);
}

// 
int create_process(ProcessQueue* queue, int pid, int life_time)
{
    Node* node;
    PCB pcb;
    int result;

    if (queue == NULL) {
        return EINVAL;
    }

    node = malloc(sizeof(*node));
    if (node == NULL) {
        return ENOMEM;
    }

    init_pcb(&pcb, pid, life_time);
    init_node(node, &pcb);

    // main.c-n begiratu 0 ez badu bueltatzen akatsen bat egon da, beraz berriro
    // prozesua sortzen saiatu beharko litzateke
    result = pthread_mutex_lock(&queue->mutex);
    if (result != 0) {
        free(node);
        return result;
    }

    // Hutsa badagao, head-era atzitu
    if (queue->tail == NULL) {
        queue->head = node;
    // Kontrakoan, bukaeran gehitu
    } else {
        queue->tail->next = node;
    }

    queue->tail = node;
    queue->size++;

    // lock egitean egin beharreko berdina, main.c-n akatsekin adi
    result = pthread_mutex_unlock(&queue->mutex);
    if (result != 0) {
        return result;
    }

    return 0;
}

// Ilaratik prozesua ateratzeko
int dequeue_process(ProcessQueue* queue, PCB* pcb)
{
    Node* node;
    int result;

    if (queue == NULL || pcb == NULL) {
        return EINVAL;
    }

    result = pthread_mutex_lock(&queue->mutex);
    if (result != 0) {
        return result;
    }

    // Ilara hutsa badago, mutex askatu eta itzuli errore kodea
    node = queue->head;
    if (node == NULL) {
        pthread_mutex_unlock(&queue->mutex);
        return ENOENT;
    }

    // Ilarako elementu bakarra borratu behar den prozesua bada
    queue->head = node->next;
    if (queue->head == NULL) {
        queue->tail = NULL;
    }
    queue->size--;
    *pcb = node->pcb;

    result = pthread_mutex_unlock(&queue->mutex);
    free(node);

    return result;
}
