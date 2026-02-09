#ifndef IPC_H
#define IPC_H

#include "common.h"
#include "data.h"

typedef struct {
    int initialized;                   //initialization flag
    DialogEntry dialogs[MAX_DIALOGS];  //dialog table
    Msg         msg_queue[Q_CAPACITY]; //ring buffer of messages
    int head;               //head index of queue   
    int tail;               //tail index of queue
    int msg_count;          //number of messages in queue   
} Shared;

extern Shared *g_shm;
extern int g_shm_id;    //shared memory id
extern int g_sem_id;    //semaphore set id

void ipc_init(void);
void ipc_cleanup(void);
void ipc_destroy_all(void);   

#endif
