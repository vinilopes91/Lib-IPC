#ifndef ipc_h__
#define ipc_h__

#define MAX_REGISTRY 50
#define MEM_SIZE 131072
#define MSG_LENGTH 512
#define MAX_BUFFER ((MEM_SIZE - sizeof(sem_t) - sizeof(int)) / sizeof(message_data)) / 2

typedef unsigned char ipc_message[MSG_LENGTH];

typedef struct message_data
{
    ipc_message message;
    pthread_t dest_id;
    pthread_t source_id;
} message_data;

int initSM();
int sendA(pthread_t dest_id, ipc_message message);
int sendS(pthread_t dest_id, ipc_message message);
int receiveA(pthread_t source_id, ipc_message message);
int receiveS(pthread_t source_id, ipc_message message);
int is_emptyA(void);

#endif  // ipc_h__
