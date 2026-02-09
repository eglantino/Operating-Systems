#include "dialog.h"

//Returns the index of the dialog with dialog_id, or -1 if not found
int find_dialog_slot(int dialog_id) {
    for (int i = 0; i < MAX_DIALOGS; i++) {
        if (g_shm->dialogs[i].used &&
            g_shm->dialogs[i].dialog_id == dialog_id) {
            return i;
        }
    }
    return -1;
}

//ψreates new dialog if not exists and returns its index, or -1 if no free slot
int create_dialog(int dialog_id) {
    int index = find_dialog_slot(dialog_id);
    if (index != -1)
        return index;

    //find free slot
    for (int i = 0; i < MAX_DIALOGS; i++) {
        if (g_shm->dialogs[i].used == 0) {
            g_shm->dialogs[i].used = 1;
            g_shm->dialogs[i].dialog_id = dialog_id;
            g_shm->dialogs[i].participant_count = 0;
            memset(g_shm->dialogs[i].participants, 0,
                   sizeof(g_shm->dialogs[i].participants));
            return i;
        }
    }
    return -1; //  no free slot
}

int dialog_join(int dialog_id, pid_t participant_pid) {
    int slot = find_dialog_slot(dialog_id);
    if (slot == -1) return -1; // dialog not found

    DialogEntry *dialog = &g_shm->dialogs[slot];

    //if already joined, return slot
    for (int i = 0; i < MAX_PARTICIPANTS; i++) {
        if (dialog->participants[i] == participant_pid)
            return i; 
    }

    //find free participant slot
    for (int i = 0; i < MAX_PARTICIPANTS; i++) {
        if (dialog->participants[i] == 0) {
            dialog->participants[i] = participant_pid;
            dialog->participant_count++;
            //The returned slot is used 
            //as the reader slot for bitmasking messages in queue subsystem
            return i;
        }
    }

    return -1; // dialog full
}

//returns readers id for bitmask, or -1 if not found
int dialog_slot_of(int dialog_id, pid_t participant_pid) {
    int slot = find_dialog_slot(dialog_id);
    if (slot == -1) return -1;

    DialogEntry *dialog = &g_shm->dialogs[slot];

    for (int i = 0; i < MAX_PARTICIPANTS; i++) {
        if (dialog->participants[i] == participant_pid)
            return i;
    }

    return -1;
}

//removes participant from dialog
int dialog_leave(int dialog_id, pid_t participant_pid) {
    int slot = find_dialog_slot(dialog_id);
    if (slot == -1) return -1;

    DialogEntry *dialog = &g_shm->dialogs[slot];
    //find participant and remove
    for (int i = 0; i < MAX_PARTICIPANTS; i++) {
        if (dialog->participants[i] == participant_pid) {
            dialog->participants[i] = 0;
            dialog->participant_count--;

            if (dialog->participant_count == 0) {
                dialog->used = 0;
                dialog->dialog_id = 0;
                memset(dialog->participants, 0,
                       sizeof(dialog->participants));
            }
            return 0;
        }
    }

    return -1;
}
