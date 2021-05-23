#ifndef ipc_h__
#define ipc_h__

#define MSG_LENGTH 5

typedef unsigned char ipc_message[MSG_LENGTH];

typedef struct message_data message_data;

int initSM();
int sendS(int shmid, ipc_message message);
int receiveS(int shmid, ipc_message message);
int sendA(int shmid, ipc_message message);
int receiveA(int shmid, ipc_message message);

#endif  // ipc_h__
