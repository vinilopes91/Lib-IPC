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
#include <stdint.h>
#include "ipc.h"

void *handle_thread(void *ptr);

int main(void)
{
    int smid = initSM();
    pthread_t thread;
    ipc_message teste = "abcde";

    pthread_create(&thread, NULL, handle_thread, (void *)(intptr_t)smid);

    sleep(2);
    sendS(smid, teste);

    pthread_join(thread, NULL);

    return 0;
}

void *handle_thread(void *ptr)
{
    int smid = (intptr_t) ptr;
    ipc_message msg;

    printf("Esperando receber mensagem\n");
    receiveS(smid, msg);
    printf("Mensagem recebida: %s\n", (char *)msg);

    pthread_exit(NULL);
}
