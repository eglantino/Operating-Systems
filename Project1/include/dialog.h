#ifndef DIALOG_H
#define DIALOG_H

#include "common.h"
#include "data.h"
#include "ipc.h"

//finds dialog slot by id in shared memory
int find_dialog_slot(int dialog_id);

//creates a new dialog or returns existing one
int create_dialog(int dialog_id);

//add a participant to dialog and returns slot index
int dialog_join(int dialog_id, pid_t participant_pid);

//returns participant slot index for given pid
int dialog_slot_of(int dialog_id, pid_t participant_pid);

//removes a participant from dialog
int dialog_leave(int dialog_id, pid_t participant_pid);

#endif
