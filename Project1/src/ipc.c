#include "ipc.h"

Shared *g_shm   = NULL;
int     g_shm_id = -1;
int     g_sem_id = -1;

union semun {
    int              val;
    struct semid_ds *buf;
    unsigned short  *array;
};

//creates and initializes all semaphores
static void sem_init_all(void) {
    g_sem_id = semget((key_t)SEM_KEY, 3, IPC_CREAT | 0600);
    if (g_sem_id == -1)
        DIE("semget");

    union semun arg;

    //all positions are free initially
    arg.val = Q_CAPACITY;
    if (semctl(g_sem_id, 0, SETVAL, arg) == -1)
        DIE("semctl spaces");

    //binary semaphore for mutual exclusion is unlocked initially
    arg.val = 1;
    if (semctl(g_sem_id, 1, SETVAL, arg) == -1)
        DIE("semctl mutex");

    //no messages at the beginning
    arg.val = 0;
    if (semctl(g_sem_id, 2, SETVAL, arg) == -1)
        DIE("semctl items");
}

//destroys all semaphores
static void sem_destroy_all(void) {
    if (g_sem_id != -1) {
        if (semctl(g_sem_id, 0, IPC_RMID, 0) == -1)
            perror("semctl IPC_RMID");
        g_sem_id = -1;
    }
}

//creates and initializes shared memory segment and attaches it to g_shm
//if first time, initializes all fields
static void shm_init(void) {
    g_shm_id = shmget((key_t)SHM_KEY, sizeof(Shared), 0600 | IPC_CREAT);
    if (g_shm_id == -1)
        DIE("shmget");

    void *ptr = shmat(g_shm_id, NULL, 0);
    if (ptr == (void *)-1)
        DIE("shmat");

    g_shm = (Shared *)ptr;

    //check if first time initialization
    if (g_shm->initialized != 1) {
        printf("[ipc] First-time initialization of shared memory.\n");

        memset(g_shm, 0, sizeof(Shared));
        g_shm->initialized = 1;

        //initialize dialogs and message queue pointers
        g_shm->head = 0;
        g_shm->tail = 0;
        g_shm->msg_count = 0;
        for (int i = 0; i < MAX_DIALOGS; i++) {
            g_shm->dialogs[i].used = 0;
            g_shm->dialogs[i].dialog_id = 0;
            g_shm->dialogs[i].participant_count = 0;
            memset(g_shm->dialogs[i].participants, 0,
                   sizeof(g_shm->dialogs[i].participants));
        }
        for (int i = 0; i < Q_CAPACITY; i++) {
            g_shm->msg_queue[i].used = 0;
            g_shm->msg_queue[i].dialog_id = 0;
            g_shm->msg_queue[i].participant_snapshot = 0;
            g_shm->msg_queue[i].readers_left = 0;
            g_shm->msg_queue[i].read_mask = 0;
            g_shm->msg_queue[i].len = 0;
            memset(g_shm->msg_queue[i].text, 0, MAX_TXT);
        }

    } else {
        printf("[ipc] Shared memory already initialized.\n");
    }
}

//called from any process that wants to use IPC and assures shared memory and semaphores are ready
void ipc_init(void) {
    shm_init();
    sem_init_all();
}

//disconnects process from shared memory
void ipc_cleanup(void) {
    if (g_shm != NULL) {
        if (shmdt((void *)g_shm) == -1)
            perror("shmdt");
        g_shm = NULL;
    }
}

//destroys IPC resources (shared memory segment and semaphores),so 
//should be called only by admin process at termination
void ipc_destroy_all(void) {
    if (g_shm_id != -1) {
        if (shmctl(g_shm_id, IPC_RMID, 0) == -1)
            perror("shmctl IPC_RMID");
        g_shm_id = -1;
    }
    sem_destroy_all();
}
