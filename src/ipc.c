#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/shm.h>
#include <string.h>
#include <semaphore.h>

#define MAX_BUFFER 10
#define MSG_LENGTH 5

typedef unsigned char ipc_message[MSG_LENGTH];

typedef struct message_data
{
    ipc_message message;
    int read;
} message_data;

struct shared_area
{
    sem_t mutex_process;
    sem_t mutex_threads;
    int flag; // 1 em uso; 0 livre;
    int buffer_size;
    int rear;
    int front;
    message_data messageS;
    message_data buffer[MAX_BUFFER];
};

int initSM()
{
    int shmid;
    struct shared_area *shared_area_ptr;
    void *shared_memory = (void *)0;
    key_t shm_key = 5671; // gerar aleatório? receber por parâmetro?

    shmid = shmget(shm_key, sizeof(struct shared_area), 0666 | IPC_CREAT);

    if (shmid == -1)
    {
        printf("Falha ao iniciar\n");
        return -1;
    }

    shared_memory = shmat(shmid, (void *)0, 0);
    shared_area_ptr = (struct shared_area *)shared_memory;

    if (sem_init((sem_t *)&shared_area_ptr->mutex_process, 1, 1) != 0)
    { // Semáforo das threads
        printf("sem_init process falhou\n");
        return -1;
    }
    if (sem_init((sem_t *)&shared_area_ptr->mutex_threads, 0, 1) != 0)
    { // Semáforo das threads
        printf("sem_init threads falhou\n");
        return -1;
    }

    shared_area_ptr->buffer_size = 0;
    shared_area_ptr->front = 0;
    shared_area_ptr->rear = 0;
    shared_area_ptr->flag = 0;
    shared_area_ptr->messageS.read = 1;

    return shmid;
}

int sendS(int shmid, ipc_message message)
{
    struct shared_area *shared_area_ptr;
    void *shared_memory = (void *)0;
    int write = 0;
    int sem_value;

    shared_memory = shmat(shmid, (void *)0, 0);
    shared_area_ptr = (struct shared_area *)shared_memory;

    while (write == 0)
    {
        // Início área crítica
        sem_wait((sem_t *)&shared_area_ptr->mutex_threads);
        sem_wait((sem_t *)&shared_area_ptr->mutex_process);

        if (shared_area_ptr->messageS.read == 1)
        {
            shared_area_ptr->messageS.read = 0;
            printf("[sendS]shared_area_ptr->messageS.read = %d\n\n", shared_area_ptr->messageS.read);
            strcpy(shared_area_ptr->messageS.message, (char *)message);
            write = 1;
        }

        sem_post((sem_t *)&shared_area_ptr->mutex_threads);
        sem_post((sem_t *)&shared_area_ptr->mutex_process);
        // Fim área crítica
    }

    printf("aguardando leitura da msg\n\n");
    printf("shared_area_ptr->messageS.read = %d\n", shared_area_ptr->messageS.read);
    while(shared_area_ptr->messageS.read == 0);
    printf("msg lida\n\n");
    printf("shared_area_ptr->messageS.read = %d\n", shared_area_ptr->messageS.read);

    return 0;
}

int receiveS(int shmid, ipc_message message)
{
    struct shared_area *shared_area_ptr;
    void *shared_memory = (void *)0;
    int read = 0;

    shared_memory = shmat(shmid, (void *)0, 0);
    shared_area_ptr = (struct shared_area *)shared_memory;

    int sem_value;
    while (read == 0)
    {
        sem_wait((sem_t *)&shared_area_ptr->mutex_threads);
        sem_wait((sem_t *)&shared_area_ptr->mutex_process);
        if (shared_area_ptr->messageS.read == 0)
        {
            printf("IADUHUIDHAWUIDHUIAHDWUIAD");
            shared_area_ptr->messageS.read = 1;
            strcpy((char *)message, shared_area_ptr->messageS.message);
            read = 1;
        }
        sem_post((sem_t *)&shared_area_ptr->mutex_threads);
        sem_post((sem_t *)&shared_area_ptr->mutex_process);
        // Fim área crítica
    }

    printf("aguardando envio da mensagem da msg\n\n");
    while(shared_area_ptr->messageS.read == 1);
    printf("msg recebida\n\n");

    return 0;
}

/* Compilação
 * gcc -c -Wall -Werror -fpic ipc.c
 * gcc -shared -o libipc.so ipc.o
 * gcc -L/home/username/ipc -Wall -o test main.c -lipc
 * export LD_LIBRARY_PATH=/home/username/ipc:$LD_LIBRARY_PATH
*/
