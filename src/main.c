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

int main(void)
{
    initSM(5648);
    pthread_t thread;
    ipc_message message = "523xp";

    pthread_create(&thread, NULL, handle_thread, (void *)(pthread_t)pthread_self());

    sleep(2);

    sendS(thread, message);

    pthread_join(thread, NULL);

    return 0;
}

void *handle_thread(void *ptr)
{
    pthread_t thread_id = (pthread_t) ptr;
    ipc_message msg;

    receiveS(thread_id, msg);

    printf("Mensagem lida pela thread: %s\n", (char *) msg);

    pthread_exit(NULL);
}
