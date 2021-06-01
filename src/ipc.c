#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/shm.h>
#include <string.h>
#include <semaphore.h>
#include "ipc.h"

typedef struct lista
{
    int qtd;
    message_data buffer[MAX_BUFFER];
} lista;

struct shared_area
{
    sem_t mutex_buffer;
    lista list;
};

int find_index(lista *li, pthread_t source_id, pthread_t dest_id);
int push(lista *li, pthread_t source_id, pthread_t dest_id, ipc_message message);
int remove_message(lista *li, pthread_t source_id, pthread_t dest_id);

struct shared_area *shared_area_ptr;

void initSM(key_t shm_key)
{
    int shmid;
    void *shared_memory = (void *)0;

    shmid = shmget(shm_key, MEM_SIZE, 0666 | IPC_CREAT);

    if (shmid == -1)
    {
        printf("Falha ao iniciar\n");
        return;
    }

    shared_memory = shmat(shmid, (void *)0, 0);
    shared_area_ptr = (struct shared_area *)shared_memory;

    if (sem_init((sem_t *)&shared_area_ptr->mutex_buffer, 1, 1) != 0)
    {
        printf("sem_init mutex_buffer falhou\n");
        return;
    }

    shared_area_ptr->list.qtd = 0;
}

int sendS(pthread_t dest_id, ipc_message message)
{
    int write = 0;
    int msg_write_index;

    sem_wait((sem_t *)&shared_area_ptr->mutex_buffer);
    int msg_index = find_index(&shared_area_ptr->list, pthread_self(), dest_id);
    if (msg_index != -1) // Verifica se existe alguma thread esperando que seja enviada uma msg
    {
        strcpy((char *)shared_area_ptr->list.buffer[msg_index].message, (char *)message);
        sem_post((sem_t *)&shared_area_ptr->list.buffer[msg_index].mutex_sync);
    }
    else if (shared_area_ptr->list.qtd == MAX_BUFFER)
    {
        sem_post((sem_t *)&shared_area_ptr->mutex_buffer);
        return -1;
    }
    else if (shared_area_ptr->list.qtd < MAX_BUFFER)
    {
        msg_write_index = push(&shared_area_ptr->list, pthread_self(), dest_id, message);
        if (msg_write_index != -1)
        {
            write = 1;
        }
        else
        {
            printf("[sendS] Falha ao armazenar mensagem\n");
        }
    }
    sem_post((sem_t *)&shared_area_ptr->mutex_buffer);

    if (write == 1)
    {
        printf("[sendS] Aguardando leitura da msg\n");
        sem_wait((sem_t *)&shared_area_ptr->list.buffer[msg_write_index].mutex_sync); // Trava a thread até receiveS da msg escrita
    }

    return 0;
}

int receiveS(pthread_t source_id, ipc_message message) // source_id = (pid ou tid)
{
    int write = 0;
    int msg_write_index;

    sem_wait((sem_t *)&shared_area_ptr->mutex_buffer);
    int msg_index = find_index(&shared_area_ptr->list, source_id, pthread_self());
    if (msg_index != -1) // Verifica se existe alguma thread esperando que seja lida alguma msg
    {
        strcpy((char *)message, (char *)shared_area_ptr->list.buffer[msg_index].message);
        sem_post((sem_t *)&shared_area_ptr->list.buffer[msg_index].mutex_sync);
        remove_message(&shared_area_ptr->list, source_id, pthread_self());
    }
    else if (shared_area_ptr->list.qtd == MAX_BUFFER) // Não achou msg no buffer e o buffer esta cheio
    {
        sem_post((sem_t *)&shared_area_ptr->mutex_buffer);
        return -1;
    }
    else if (shared_area_ptr->list.qtd < MAX_BUFFER) // Registra uma msg no buffer para ser lida
    {
        msg_write_index = push(&shared_area_ptr->list, source_id, pthread_self(), message);
        if (msg_write_index != -1)
        {
            write = 1;
        }
        else
        {
            printf("[receiveS] Falha ao armazenar message_data\n");
        }
    }
    sem_post((sem_t *)&shared_area_ptr->mutex_buffer);

    if (write == 1)
    {
        printf("[receiveS] Aguardando envio da msg %lu\n", pthread_self());
        sem_wait((sem_t *)&shared_area_ptr->list.buffer[msg_write_index].mutex_sync); // Trava a thread até receiveS da msg escrita

        sem_wait((sem_t *)&shared_area_ptr->mutex_buffer);
        int msg_index = find_index(&shared_area_ptr->list, source_id, pthread_self());
        strcpy((char *)message, (char *)shared_area_ptr->list.buffer[msg_index].message);
        remove_message(&shared_area_ptr->list, source_id, pthread_self());
        sem_post((sem_t *)&shared_area_ptr->mutex_buffer);
    }
    return 0;
}

int push(lista *li, pthread_t source_id, pthread_t dest_id, ipc_message message)
{
    if (li == NULL)
    {
        return -1;
    }

    if (li->qtd == MAX_BUFFER) // lista cheia
    {
        return -1;
    }

    int index = li->qtd;

    li->buffer[li->qtd].dest_id = dest_id;
    strcpy((char *)li->buffer[li->qtd].message, (char *)message);
    li->buffer[li->qtd].source_id = source_id;

    if (sem_init((sem_t *)&li->buffer[li->qtd].mutex_sync, 1, 0) != 0)
    {
        printf("sem_init message_data falhou\n");
        return -1;
    }

    li->qtd++;

    return index;
}

int remove_message(lista *li, pthread_t source_id, pthread_t dest_id)
{
    if (li == NULL)
    {
        return -1;
    }
    if (li->qtd == 0)
    {
        return -1;
    }

    int k, i = 0;

    while (i < li->qtd && li->buffer[i].dest_id != dest_id && li->buffer[i].source_id != source_id)
    {
        i++;
    }

    if (i == li->qtd) //elemento nao encontrado
    {
        return 0;
    }

    for (k = i; k < li->qtd - 1; k++)
    {
        li->buffer[k].dest_id = li->buffer[k + 1].dest_id;
        strcpy((char *)li->buffer[k].message, (char *)li->buffer[k + 1].message);
        li->buffer[k].mutex_sync = li->buffer[k + 1].mutex_sync;
        li->buffer[k].source_id = li->buffer[k + 1].source_id;
    }

    li->qtd--;

    return 0;
}

int find_index(lista *li, pthread_t source_id, pthread_t dest_id)
{
    if (li == NULL)
    {
        return -1;
    }
    if (li->qtd == 0)
    {
        return -1;
    }

    int index = 0;

    while (index < li->qtd)
    {
        if (li->buffer[index].source_id == source_id && li->buffer[index].dest_id == dest_id)
        {
            break;
        }
        index++;
    }
    if (index == li->qtd)
    {
        return -1;
    }

    return index;
}
