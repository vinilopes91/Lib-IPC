#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <semaphore.h>
#include "ipc.h"

void *handle_thread(void *ptr);

#define MAX_THREADS MAX_REGISTRY - 1

int main(void)
{
    if (initSM() == -1)
    {
        printf("Falha thread principal\n");
        return 0;
    }

    pthread_t thread[MAX_THREADS];
    ipc_message message;

    for (int i = 0; i < MAX_THREADS; i++)
    {
        pthread_create(&thread[i], NULL, handle_thread, (void *)(pthread_t)pthread_self());
    }

    for (int i = 0; i < MAX_THREADS; i++)
    {
        sprintf((char *)message, "msg=%d", i);
        sendS(thread[i], message);
	    //sendA(thread[i], message)
    }

    for (int i = 0; i < MAX_THREADS; i++)
    {
        pthread_join(thread[i], NULL);
    }

    printf("\nFim do programa principal\n");

    return 0;
}

void *handle_thread(void *ptr)
{
    initSM();
    pthread_t thread_id = (pthread_t) ptr;
    ipc_message msg;

    if (receiveS(thread_id, msg) == -1)
    {
        printf("Buffer cheio\n");
    }
    else
    {
        printf("Mensagem lida pela thread: %s\n", (char *) msg);
    }

    pthread_exit(NULL);
}
