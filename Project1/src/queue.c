#include "queue.h"
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

//Message queue implementation using shared memory 

//The queue is a circular buffer synchronized with semaphores(systemV)
//Each message is read exactly once by each participant in the dialog
//Synchronization is done with 3 semaphores:
//sem[0](spaces): counts free slots in the queue
//sem[1](mutex): binary semaphore for mutual exclusion on shared memory access
//sem[2](items): pending deliveries
//items is gloval.so the receiver may wake up and still not find a message for its dialog/slot

static int semop_retry(struct sembuf *op) { //this prevents failurs when blocking semop is interrupted by signal
    for (;;) {
        if (semop(g_sem_id, op, 1) == 0) return 0;
        if (errno == EINTR) continue;
        return -1;
    }
}

//find message ready to be received for specific user based on dialog id and reader slot
static int q_find_msg_for_reader(int dialog_id, int my_slot) {
    int index = g_shm->head;
    int checked = 0;

    while (checked < g_shm->msg_count) {
        Msg *m = &g_shm->msg_queue[index];

        if (m->used &&
            m->dialog_id == dialog_id &&
            (m->read_mask & (1u << my_slot)) == 0) {
            return index;
        }

        index = (index + 1) % Q_CAPACITY;
        checked++;
    }
    return -1;
}

//mark message as read by participant slot my_slot
static void q_mark_msg_as_read(Msg *m, int my_slot) {
    uint32_t bit = (1u << my_slot);

    if ((m->read_mask & bit) == 0) {
        m->read_mask |= bit;
        m->readers_left--;
    }
}

//garbage collection from the head of the queue(only after all participants have read the message)
static int gc_head(void) {
    int freed = 0;

    while (g_shm->msg_count > 0) {
        Msg *m = &g_shm->msg_queue[g_shm->head];

        if (m->used && m->readers_left == 0) {
            m->used = 0;
            g_shm->head = (g_shm->head + 1) % Q_CAPACITY;
            g_shm->msg_count--;
            freed++;
        } else {
            break;
        }
    }
    return freed;
}

int q_enqueue(int dialog_id, pid_t sender, const char *message) {
    //P(spaces): wait for free slot
    struct sembuf P_spaces = {0, -1, 0};
    if (semop_retry(&P_spaces) == -1) DIE("semop P_spaces");

    //P(mutex): enter critical section
    struct sembuf P_mutex = {1, -1, 0};
    if (semop_retry(&P_mutex) == -1) DIE("semop P_mutex");

    int ds_index = find_dialog_slot(dialog_id);
    if (ds_index < 0) {
        //release and undo space reservation
        struct sembuf V_mutex = {1, +1, 0};
        semop_retry(&V_mutex);
        struct sembuf V_spaces = {0, +1, 0};
        semop_retry(&V_spaces);
        return -1;
    }

    DialogEntry *de = &g_shm->dialogs[ds_index];
    if (de->participant_count <= 0) {
        //no receivers means nothing meaningful to enqueue
        struct sembuf V_mutex = {1, +1, 0};
        semop_retry(&V_mutex);
        struct sembuf V_spaces = {0, +1, 0};
        semop_retry(&V_spaces);
        return -1;
    }

    //write at tail
    int pos = g_shm->tail;
    Msg *m = &g_shm->msg_queue[pos];

    m->used = 1;
    m->dialog_id = dialog_id;
    m->sender_pid = sender;
    m->participant_snapshot = de->participant_count;
    m->readers_left = de->participant_count;
    m->read_mask = 0;

    strncpy(m->text, message, MAX_TXT);
    m->text[MAX_TXT - 1] = '\0';

    g_shm->tail = (g_shm->tail + 1) % Q_CAPACITY;
    g_shm->msg_count++;

    //V(mutex): leave critical section
    struct sembuf V_mutex = {1, +1, 0};
    if (semop_retry(&V_mutex) == -1) DIE("semop V_mutex");

    //V(items) by N receivers: one read-event per receiver
    struct sembuf V_items = {2, de->participant_count, 0};
    if (semop_retry(&V_items) == -1) DIE("semop V_items");

    return 0;
}

int q_recv_block(int dialog_id, pid_t me, char *out_text) {
    int my_slot = dialog_slot_of(dialog_id, me);
    if (my_slot < 0) return -1; // not a participant

    for (;;) {
        //P(items): wait until there exists at least one remaining unread delivery globally
        struct sembuf P_items = {2, -1, 0};
        if (semop_retry(&P_items) == -1) DIE("semop P_items");

        //P(mutex): inspect and update shared queue safely
        struct sembuf P_mutex = {1, -1, 0};
        if (semop_retry(&P_mutex) == -1) DIE("semop P_mutex");

        int msg_index = q_find_msg_for_reader(dialog_id, my_slot);
        if (msg_index >= 0) {
            Msg *m = &g_shm->msg_queue[msg_index];

            q_mark_msg_as_read(m, my_slot);

            //Copy out (ensure termination)
            strncpy(out_text, m->text, MAX_TXT);
            out_text[MAX_TXT - 1] = '\0';

            //Free any fully-delivered messages from the head
            int freed = gc_head();

            //V(mutex)
            struct sembuf V_mutex = {1, +1, 0};
            if (semop_retry(&V_mutex) == -1) DIE("semop V_mutex");

            //V(spaces) by how many message slots were actually freed
            if (freed > 0) {
                struct sembuf V_spaces = {0, freed, 0};
                if (semop_retry(&V_spaces) == -1) DIE("semop V_spaces");
            }

            return 0;
        }

        struct sembuf V_mutex = {1, +1, 0};
        if (semop_retry(&V_mutex) == -1) DIE("semop V_mutex");

        struct sembuf V_items = {2, +1, 0};
        if (semop_retry(&V_items) == -1) DIE("semop V_items");

        usleep(10 * 1000); 
    }
}



