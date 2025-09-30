#ifndef MESSAGING_H
    #define MESSAGING_H
    #define ERROR -1
    #define READER 0
    #define WRITER 1
    #define MAX_PROCESSES 20000
    #define SIZEofBUFF 20
    #include <semaphore.h>
    #include <time.h>

    typedef struct{
        int  	custid;
        char 	LastName[SIZEofBUFF];
        char 	FirstName[SIZEofBUFF];
        int	balance;
    } MyRecord;

    typedef struct{     //This will be used in the array of locks to check
        int Index;      //The index of the lock in the shared memory
        int TimesChanged;       //The current number of times the lock has changed, before a process wait to a lock it checks if it has changed
    }LockToCheck;

    typedef struct{
        int startingId;
        int endingId;
        int WriterReader;       //This is used to check if the lock is a reader or a writer
        bool valid;     //This is used to check if the lock is being used
        sem_t semaphore;
        int NumberOfBlocked;        //This is used to post the semaphore the correct number of times
        bool ProcessLockingLeft;        //This is used to check if the process that locked the lock has termianted
        int timesChanged;       //This is used to check if the lock has changed while a process was waiting
    }RangeLock;     //This is the lock that will be used in the shared memory

    typedef struct{
        int numActiveReaders;
        int numActriveWriters;
        int numRecordsProecessed;
        int numReaders;
        int numWriters;
        double Readersime;
        double WritersTime;
        double MaxWaiting;
        int ActiveReaders[MAX_PROCESSES];
        int ActiveWriters[MAX_PROCESSES];
        sem_t semaphore;        //This is the semaphore that will be used to lock the shared memory
        RangeLock SharedRangeLocks[2*MAX_PROCESSES];
    }SharedData;    //This is the shared memory as described in the project
#endif