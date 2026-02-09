#ifndef QUEUE_H
#define QUEUE_H

#include "common.h"
#include "data.h"
#include "ipc.h"
#include "dialog.h"

//enqueue a message to the queue
//returns 0 on success, -1 on errorq
int q_enqueue(int dialog_id,pid_t sender,const char *message);

//blocking receive:waits until a message is available
//returns 0 on success, -1 on error
int q_recv_block(int dialog_id,pid_t me,char* out_text);

#endif