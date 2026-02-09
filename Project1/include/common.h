#ifndef COMMON_H
#define COMMON_H

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>

//keys for shared memory and semaphores
#define SHM_KEY 0x12345     //shared memrory key
#define SEM_KEY 0x54321     //semaphore key

#define MAX_DIALOGS 16      //maximum number of dialogs
#define MAX_TXT 256     //maximum length of a message
#define MAX_PARTICIPANTS 16 //maximum number of participants in a dialog
#define Q_CAPACITY 128      //capacity of the message queue

//an ola pane skata 
#define DIE(msg)          \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

#endif 