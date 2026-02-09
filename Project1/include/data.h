#ifndef DATA_H
#define DATA_H

#include "common.h"

typedef struct {
    int used;           //used slot flag     
    int dialog_id;          
    pid_t sender_pid;
    int participant_snapshot;   //prticipant count at send time
    int readers_left;           //how many have yet to read
    u_int32_t read_mask;        //bitmask of readers
    size_t len;           //length of text
    char text[MAX_TXT];      //message text
} Msg;

typedef struct {
    int used;                    
    int dialog_id;               
    int participant_count;       
    pid_t participants[MAX_PARTICIPANTS];   //participant pids
} DialogEntry;

#endif
