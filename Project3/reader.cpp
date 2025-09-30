#include <iostream>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
# include <sys/times.h>

#include "Shared.h"

using namespace std;

int main (int argc, char * argv[]){
    if (argc != 9){     //Since our program only accpets calls as described in the assignment
        cout << "Wrong number of arguments!\n";
        return ERROR;
    }
    char * dataFile=NULL, *recs = NULL, *timeString = NULL, *shmidString=NULL; 
    int recid1, recid2;
    for (int i = 1 ; i < argc ; i +=2){
        //Simple reading of the arguments
        if(strcmp(argv[i],"-f") == 0 ){
            if ( ( dataFile = (char*)malloc(sizeof(char)*strlen(argv[i+1])+1) ) ==NULL){
                cout << "Could not allocate memory\n";
                return ERROR;
            }
            strcpy(dataFile,argv[i+1]);
        }
        else if(strcmp(argv[i],"-l") == 0 ){
            if ( ( recs = (char*)malloc(sizeof(char)*strlen(argv[i+1])+1) ) ==NULL){
                cout << "Could not allocate memory\n";
                return ERROR;
            }
            strcpy(recs,argv[i+1]);
            int result = sscanf(recs,"%d,%d",&recid1,&recid2);
            if (result == 2)
                continue;
            sscanf(recs,"%d",&recid1);
            recid2 = recid1;
        }
        else if(strcmp(argv[i],"-d") == 0 ){
            if ( ( timeString = (char*)malloc(sizeof(char)*strlen(argv[i+1])+1) ) ==NULL){
                cout << "Could not allocate memory\n";
                return ERROR;
            }
            strcpy(timeString,argv[i+1]);
        }
        else if(strcmp(argv[i],"-s") == 0 ){
            if ( ( shmidString = (char*)malloc(sizeof(char)*strlen(argv[i+1])+1) ) ==NULL){
                cout << "Could not allocate memory\n";
                return ERROR;
            }
            strcpy(shmidString,argv[i+1]);
        }
        else{
            //Invalid flag
            cout << "Invalid input as argument\n";
            return ERROR;
        }
    }
    //Open shared memory
    

    int shm_fd = shm_open(shmidString, O_RDWR, 0666);
    if (shm_fd == -1){
        cout << "shm_open failed" << endl;
        return ERROR;
    }
    int size = sizeof(SharedData);
    SharedData * sharedData = (SharedData *)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (sharedData == MAP_FAILED){
        cout << "mmap failed" << endl;
        return ERROR;
    }
    LockToCheck LocksToCheck[2*MAX_PROCESSES];    //This is used to check if a lock has changed while a process was waiting
    int NumberOfLocksToCheck;
    double t1 , t2;
    struct tms tb1,tb2;
    double ticspersec ;
    ticspersec = ( double ) sysconf ( _SC_CLK_TCK );

    //Add a lock to shared memoery
    sem_wait(&sharedData->semaphore);       //Lock shared memory
    NumberOfLocksToCheck = 0;
    int j,LockPosition=ERROR;
    for ( j = 0 ; j < 2*MAX_PROCESSES ;j++){
        if (!sharedData->SharedRangeLocks[j].valid && LockPosition == ERROR){   //Find the first invalid lock and use it
            sharedData->SharedRangeLocks[j].valid = true;
            sharedData->SharedRangeLocks[j].startingId = recid1;
            sharedData->SharedRangeLocks[j].endingId = recid2;
            sharedData->SharedRangeLocks[j].WriterReader = READER;
            sharedData->SharedRangeLocks[j].NumberOfBlocked = 0;
            sharedData->SharedRangeLocks[j].ProcessLockingLeft = false;
            sharedData->SharedRangeLocks[j].timesChanged++;
            sem_init(&sharedData->SharedRangeLocks[j].semaphore, 1, 0);
            LockPosition = j;
            cout << "Reader " << getpid() << " is trying to read from " << recid1 << " to " << recid2 << " and created lock " << j << endl;
            cout.flush();
        }
        else if (sharedData->SharedRangeLocks[j].valid && sharedData->SharedRangeLocks[j].WriterReader == WRITER){
            // Add all writer locks to the array of locks to check
            LocksToCheck[NumberOfLocksToCheck].Index = j;
            LocksToCheck[NumberOfLocksToCheck++].TimesChanged = sharedData->SharedRangeLocks[j].timesChanged;
        }
        
    }
    if (LockPosition==ERROR){
        cout << "Too many locks\n";
        cout.flush();
        exit(ERROR);
    }
    t1 = ( double ) times (& tb1) ;
    for (int j = 0 ; j < NumberOfLocksToCheck ; j++){       // Check all locks that are writers
        if (LocksToCheck[j].Index == LockPosition)   //Skip the lock that the process is using
            continue;
        if (LocksToCheck[j].TimesChanged != sharedData->SharedRangeLocks[LocksToCheck[j].Index].timesChanged)   //Check if the lock has changed while the process was waiting
            continue;
        if (sharedData->SharedRangeLocks[LocksToCheck[j].Index].valid ){       //Check if the lock is valid and a writer
            if (sharedData->SharedRangeLocks[LocksToCheck[j].Index].ProcessLockingLeft)
                continue;
            if (sharedData->SharedRangeLocks[LocksToCheck[j].Index].startingId >= recid1 && sharedData->SharedRangeLocks[LocksToCheck[j].Index].startingId <= recid2){      //Check if the lock is in the range of the process
                sharedData->SharedRangeLocks[LocksToCheck[j].Index].NumberOfBlocked++;
                sem_post(&sharedData->semaphore);
                cout << "Reader " << getpid() << " is waiting at lock " << LocksToCheck[j].Index << endl;
                cout.flush();
                sem_wait(&sharedData->SharedRangeLocks[LocksToCheck[j].Index].semaphore);
                sem_wait(&sharedData->semaphore);
                sharedData->SharedRangeLocks[LocksToCheck[j].Index].NumberOfBlocked--;
                if (sharedData->SharedRangeLocks[LocksToCheck[j].Index].NumberOfBlocked == 0){
                    sharedData->SharedRangeLocks[LocksToCheck[j].Index].valid = false;
                    sharedData->SharedRangeLocks[LocksToCheck[j].Index].ProcessLockingLeft = false;
                }
            }
        }
    }
    cout << "Reader " << getpid() << " has passed all locks\n";
    cout.flush();
    t2 = ( double ) times (& tb2) ;
    long seconds =  (t2 - t1) / ticspersec;
    if (sharedData->MaxWaiting < seconds)
        sharedData->MaxWaiting = seconds;
    sharedData->numActiveReaders++;
    cout << "Number of active readers is " << sharedData->numActiveReaders << endl;
    cout.flush();
    //Add process to active readers
    t1 = ( double ) times (& tb1);
    sharedData->ActiveReaders[sharedData->numActiveReaders-1] = getpid();
    sem_post(&sharedData->semaphore);
    double average=0;
    int FileDescriptor = open(dataFile, O_RDWR);
    if (FileDescriptor == -1){
        cout << "open failed" << endl;
        return ERROR;
    }
    for (int j = recid1 ; j <= recid2 ; j++){
        MyRecord record;
        pread(FileDescriptor, &record, sizeof(MyRecord), j*sizeof(MyRecord));
        average += record.balance/((double)(recid2-recid1+1));
    }
    int TimeToSleep = rand()%atoi(timeString);
    //cout << "Reader is going to sleep for " << TimeToSleep << endl;
    usleep(TimeToSleep);
    cout << "Average balance for records " << recid1 << " to " << recid2 << " is " << average << endl;
    cout.flush();
    close(FileDescriptor);
    sem_wait(&sharedData->semaphore);
    for (int j = 0 ; j < sharedData->SharedRangeLocks[LockPosition].NumberOfBlocked ; j++){
        sem_post(&sharedData->SharedRangeLocks[LockPosition].semaphore);
    }
    sharedData->SharedRangeLocks[LockPosition].ProcessLockingLeft = true;
    //Remove process from active readers
    for (int j = 0 ; j <sharedData->numActiveReaders  ; j++){
        if (sharedData->ActiveReaders[j] == getpid()){
            for (int k = j ; k < sharedData->numActiveReaders-1 ; k++){
                sharedData->ActiveReaders[k] = sharedData->ActiveReaders[k+1];
            }
            break;
        }
    }
    sharedData->numActiveReaders--;
    t2 = ( double ) times (& tb2) ;
    seconds = (t2 - t1) / ticspersec;
    sharedData->Readersime += seconds;
    sharedData->numReaders++;
    sharedData->numRecordsProecessed += (recid2-recid1+1);
    cout << "Reader " << getpid() << " has finished\n";
    cout.flush();
    sem_post(&sharedData->semaphore);

}