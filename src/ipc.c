#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <sys/shm.h>
#include <string.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include "ipc.h"

typedef struct thread_mutex_sync
{
    sem_t mutex;
    pthread_t thread_id;
} thread_sync;

typedef struct lista
{
    int qtd;
    message_data buffer[MAX_BUFFER];
    // int write;
} lista;

typedef struct lista_threads
{
    int qtd;
    thread_sync threads[MAX_REGISTRY];
} lista_threads;

struct shared_area
{
    sem_t mutex_buffer;
    lista list;
};

int find_message_index(lista *li, pthread_t source_id, pthread_t dest_id);
int push(lista *li, pthread_t source_id, pthread_t dest_id, ipc_message message);
int remove_message(lista *li, pthread_t source_id, pthread_t dest_id);
int find_thread_registry_index(lista_threads *li, pthread_t thread_id);
int rand_interval(int a, int b);

struct shared_area *shared_area_ptr;
sem_t mutex_lib;
lista_threads threads_registry;
int lib_ready = 0;


int initSM()
{
    int shmid;
    void *shared_memory = NULL;
    key_t shm_key = (key_t)rand_interval(1, 100);

    if (sem_init((sem_t *)&mutex_lib, 1, 1) != 0)
    {
        printf("sem_init mutex_lib falhou\n");
        return -1;
    }

    sem_wait((sem_t *)&mutex_lib);
    if (lib_ready == 0)
    {
        shmid = shmget(shm_key, 65536, IPC_CREAT | SHM_HUGETLB | 0666);

        if (shmid == -1)
        {
            printf("Falha ao iniciar\n");
            printf("%d\n", errno);
            perror("Error: ");
            return -1;
        }

        shared_memory = shmat(shmid, NULL, 0);
        shared_area_ptr = (struct shared_area *)shared_memory;

        if (sem_init((sem_t *)&shared_area_ptr->mutex_buffer, 1, 1) != 0)
        {
            printf("sem_init mutex_buffer falhou\n");
            return -1;
        }
	    // shared_area_ptr->list.write = 0;
        shared_area_ptr->list.qtd = 0;

        threads_registry.threads[threads_registry.qtd].thread_id = pthread_self();
        if (sem_init((sem_t *)&threads_registry.threads[threads_registry.qtd].mutex, 1, 0) != 0)
        {
            printf("Falha ao registrar thread\n");
            return -1;
        }
        threads_registry.qtd = 1;
        lib_ready = 1;
    }
    else if (shared_area_ptr == NULL)
    {
        return -1;
    }
    else
    {
        if (threads_registry.qtd == MAX_REGISTRY)
        {
            printf("Número máximo de threads registrado.\n");
            sem_post((sem_t *)&mutex_lib);
            return -1;
        }
        threads_registry.threads[threads_registry.qtd].thread_id = pthread_self();
        if (sem_init((sem_t *)&threads_registry.threads[threads_registry.qtd].mutex, 1, 0) != 0)
        {
            printf("Falha ao registrar thread\n");
            return -1;
        }
        threads_registry.qtd += 1;
    }
    sem_post((sem_t *)&mutex_lib);

    return 0;
}
int sendA(pthread_t dest_id, ipc_message message){
    
    int msg_index = find_message_index(&shared_area_ptr->list, pthread_self(), dest_id);
    if (msg_index != -1)
    {
	//if(shared_area_ptr->list.write == 0)
	//{
	//    shared_area_ptr->list.write = 1;
        strcpy((char *)shared_area_ptr->list.buffer[msg_index].message, (char *)message);
	//    shared_area_ptr->list.write = 0;
	    return 0;
	//}
	//else
	//	return -2;//está ocupado
    }
    else if(shared_area_ptr->list.qtd < MAX_BUFFER) {
	//if(shared_area_ptr->list.write == 0){
	//        shared_area_ptr->list.write = 1;	
        if(push(&shared_area_ptr->list, pthread_self(), dest_id, message) == -1){
	//            shared_area_ptr->list.write = 0;
		    return -1;
	    }
	    else{
		    //shared_area_ptr->list.write = 0;
		    return 0;
	    }
	//}
	//else
	//	return -2;//está ocupado
    }
    else {
	    return -1; // cheio
    }
}

int sendS(pthread_t dest_id, ipc_message message)
{
    int write = 0;
    int msg_write_index;

    if (find_thread_registry_index(&threads_registry, pthread_self()) == -1)
    {
        printf("[sendS] Thread não registrada\n");
        return -1;
    }

    sem_wait((sem_t *)&shared_area_ptr->mutex_buffer);
    int msg_index = find_message_index(&shared_area_ptr->list, pthread_self(), dest_id);
    if (msg_index != -1) // Verifica se existe alguma thread esperando que seja enviada uma msg
    {
        strcpy((char *)shared_area_ptr->list.buffer[msg_index].message, (char *)message);
        int thread_index = find_thread_registry_index(&threads_registry, dest_id);
        sem_post((sem_t *)&threads_registry.threads[thread_index].mutex);
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
        int thread_index = find_thread_registry_index(&threads_registry, pthread_self());
        sem_wait((sem_t *)&threads_registry.threads[thread_index].mutex); // Trava a thread até receiveS da msg escrita
    }

    return 0;
}
int receiveA(pthread_t source_id, ipc_message message)
{
   
   int msg_index = find_message_index(&shared_area_ptr->list, source_id, pthread_self());
   if (msg_index != -1)
   {
      strcpy((char *)message, (char *)shared_area_ptr->list.buffer[msg_index].message);
      remove_message(&shared_area_ptr->list, source_id, pthread_self());
      return 0;

   }
   else {
       return -1;
   }
}

int receiveS(pthread_t source_id, ipc_message message) // source_id = (pid ou tid)
{
    int write = 0;
    int msg_write_index;

    if (find_thread_registry_index(&threads_registry, pthread_self()) == -1)
    {
        printf("[receiveS] Thread não registrada\n");
        return -1;
    }

    sem_wait((sem_t *)&shared_area_ptr->mutex_buffer);
    int msg_index = find_message_index(&shared_area_ptr->list, source_id, pthread_self());
    if (msg_index != -1) // Verifica se existe alguma thread esperando que seja lida alguma msg
    {
        strcpy((char *)message, (char *)shared_area_ptr->list.buffer[msg_index].message);
        int thread_index = find_thread_registry_index(&threads_registry, source_id);
        sem_post((sem_t *)&threads_registry.threads[thread_index].mutex);
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
        int thread_index = find_thread_registry_index(&threads_registry, pthread_self());
        printf("[receiveS] Esperando sendS correspondente...\n");
        sem_wait((sem_t *)&threads_registry.threads[thread_index].mutex); // Trava a thread até sendS correspondente

        sem_wait((sem_t *)&shared_area_ptr->mutex_buffer);
        int msg_index = find_message_index(&shared_area_ptr->list, source_id, pthread_self());
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

    while (i < li->qtd && (li->buffer[i].dest_id != dest_id || li->buffer[i].source_id != source_id))
    {
        i++;
    }

    if (i == li->qtd) //elemento nao encontrado
    {
        return -1;
    }

    for (k = i; k < li->qtd - 1; k++)
    {
        li->buffer[k].dest_id = li->buffer[k + 1].dest_id;
        strcpy((char *)li->buffer[k].message, (char *)li->buffer[k + 1].message);
        li->buffer[k].source_id = li->buffer[k + 1].source_id;
    }

    li->qtd--;

    return 0;
}

int find_message_index(lista *li, pthread_t source_id, pthread_t dest_id)
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

    while (index < li->qtd && (li->buffer[index].source_id != source_id || li->buffer[index].dest_id != dest_id))
    {
        index++;
    }
    if (index == li->qtd)
    {
        return -1;
    }

    return index;
}

int find_thread_registry_index(lista_threads *li, pthread_t thread_id)
{
    if (li == NULL)
    {
        return -1;
    }

    int index = 0;

    while (index < MAX_REGISTRY && li->threads[index].thread_id != thread_id)
    {
        index++;
    }
    if (index == MAX_REGISTRY)
    {
        return -1;
    }

    return index;
}

int rand_interval(int a, int b)
{
    return rand() % (b - a + 1) + a;
}
