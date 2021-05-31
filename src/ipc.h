#ifndef ipc_h__
#define ipc_h__

#define MSG_LENGTH 5

typedef unsigned char ipc_message[MSG_LENGTH];

typedef struct message_data
{
    ipc_message message;
    pthread_t dest_id;
    pthread_t source_id;
    sem_t mutex_sync;
} message_data;

void initSM(key_t shm_key);
int sendS(pthread_t dest_id, ipc_message message);
int receiveS(pthread_t source_id, ipc_message message);


#endif  // ipc_h__
