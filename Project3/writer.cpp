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
//Program for the writer
int main (int argc, char * argv[]){
    // Print all arguments
    if (argc != 11){     //Since our program only accpets calls as described in the assignment
        cout << "Wrong number of arguments!\n";
        return ERROR;
    }
    char * dataFile=NULL, *recs = NULL, *timeString = NULL, *shmidString=NULL, *valueString=NULL; 
    int recid;
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
            recid = atoi(argv[i+1]);
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
        else if(strcmp(argv[i],"-v") == 0 ){
            if ( ( valueString = (char*)malloc(sizeof(char)*strlen(argv[i+1])+1) ) ==NULL){
                cout << "Could not allocate memory\n";
                return ERROR;
            }
            strcpy(valueString,argv[i+1]);
        }
        else{
            //Invalid flag
            cout << "Invalid input as argument\n";
            return ERROR;
        }
    }
    

    int shm_fd = shm_open(shmidString, O_RDWR, 0666);   //Open the shared memory using the id given
    if (shm_fd == -1){
        cout << "shm_open failed" << endl;
        return ERROR;
    }
    int size = sizeof(SharedData);
    //Map the shared memory
    SharedData * sharedData = (SharedData *)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (sharedData == MAP_FAILED){
        cout << "mmap failed" << endl;
        return ERROR;
    }
    LockToCheck LocksToCheck[2*MAX_PROCESSES]; //Array to store the indexes of the locks that need to be checked
    int NumberOfLocksToCheck;
    double t1 , t2;
    struct tms tb1,tb2;
    double ticspersec ;
    ticspersec = ( double ) sysconf ( _SC_CLK_TCK );    
    NumberOfLocksToCheck = 0;
    sem_wait(&sharedData->semaphore);       //Lock the shared memory
    int j,LockPosition=ERROR;
    for ( j = 0 ; j < 2*MAX_PROCESSES ;j++){
        if (!sharedData->SharedRangeLocks[j].valid && LockPosition == ERROR){       //Find the first unused lock
            sharedData->SharedRangeLocks[j].valid = true;   //Initialize the lock
            sharedData->SharedRangeLocks[j].startingId = recid;  //Set the starting id of the lock
            sharedData->SharedRangeLocks[j].endingId = recid;   // Since it is a writer the ending id is the same as the starting id
            sharedData->SharedRangeLocks[j].WriterReader = WRITER;
            sharedData->SharedRangeLocks[j].NumberOfBlocked = 0;
            sharedData->SharedRangeLocks[j].ProcessLockingLeft = false;
            sharedData->SharedRangeLocks[j].timesChanged++;
            sem_init(&sharedData->SharedRangeLocks[j].semaphore, 1, 0);
            LockPosition = j;
            cout << "Writer " << getpid() << " is trying to write to " << recid << " and created lock " << j << endl;
            cout.flush();
        }
        else if (sharedData->SharedRangeLocks[j].valid  ){   //If the lock is valid
            LocksToCheck[NumberOfLocksToCheck].Index = j;   //Add it to the array of locks to check
            LocksToCheck[NumberOfLocksToCheck++].TimesChanged = sharedData->SharedRangeLocks[j].timesChanged;
            //Also add the number of times it has changed, this is used to check if the lock has changed while a process was waiting
        }
    }
    if (LockPosition==ERROR){   //If no unused lock was found
        cout << "Too many locks\n";
        cout.flush();
        exit(ERROR);
    }
    t1 = ( double ) times (& tb1) ;     //Measure time waiting at locks
    for (int j = 0;j<NumberOfLocksToCheck ; j++){
        if (LocksToCheck[j].Index == LockPosition)      //If the lock is the lock of the process
            continue;
        if (LocksToCheck[j].TimesChanged != sharedData->SharedRangeLocks[LocksToCheck[j].Index].timesChanged) //If the lock has changed while the process was waiting at another lock
            continue;
        if (sharedData->SharedRangeLocks[LocksToCheck[j].Index].valid && sharedData->SharedRangeLocks[j].WriterReader == WRITER){
            //If the lock is valid and it is a writer
            if (sharedData->SharedRangeLocks[LocksToCheck[j].Index].ProcessLockingLeft)
                //If the process that locked the lock has termianted, then continue
                continue;
            if (sharedData->SharedRangeLocks[LocksToCheck[j].Index].startingId == recid ){
                //If the lock is for the same record as the one the process wants to write to, then process must wait
                //Keep in mind that until now the process is in the critical section of the shared memory
                sharedData->SharedRangeLocks[LocksToCheck[j].Index].NumberOfBlocked++;      //Increase the number of processes waiting at the lock
                sem_post(&sharedData->semaphore);       //Unlock the shared memory
                sem_wait(&sharedData->SharedRangeLocks[LocksToCheck[j].Index].semaphore);   //Wait at the lock
                //The process passed the lock, now it must wait at the shared memory to update the number of processes waiting at the lock
                cout << "Writer " << getpid() << " is waiting at lock " << LocksToCheck[j].Index << endl;
                cout.flush();
                sem_wait(&sharedData->semaphore);
                sharedData->SharedRangeLocks[LocksToCheck[j].Index].NumberOfBlocked--;
                if (sharedData->SharedRangeLocks[LocksToCheck[j].Index].NumberOfBlocked == 0){
                    //If no processes are waiting at the lock, then the lock is no longer valid
                    //This lock may be used by another process
                    sharedData->SharedRangeLocks[LocksToCheck[j].Index].ProcessLockingLeft = false;
                    sharedData->SharedRangeLocks[LocksToCheck[j].Index].valid = false;
                }
            }
        }
        else if (sharedData->SharedRangeLocks[LocksToCheck[j].Index].valid && sharedData->SharedRangeLocks[LocksToCheck[j].Index].WriterReader == READER){
            //Same as above but for readers
            if (sharedData->SharedRangeLocks[LocksToCheck[j].Index].ProcessLockingLeft)
                continue;
            if (sharedData->SharedRangeLocks[LocksToCheck[j].Index].startingId <= recid && sharedData->SharedRangeLocks[LocksToCheck[j].Index].endingId >= recid){
                //If the writer is writing to a record that is being read by another process
                sharedData->SharedRangeLocks[LocksToCheck[j].Index].NumberOfBlocked++;
                sem_post(&sharedData->semaphore);
                sem_wait(&sharedData->SharedRangeLocks[LocksToCheck[j].Index].semaphore);
                cout << "Writer" << getpid() << " is waiting at lock " << LocksToCheck[j].Index << endl;
                cout.flush();
                sem_wait(&sharedData->semaphore);
                sharedData->SharedRangeLocks[LocksToCheck[j].Index].NumberOfBlocked--;
                if (sharedData->SharedRangeLocks[LocksToCheck[j].Index].NumberOfBlocked == 0){
                    sharedData->SharedRangeLocks[LocksToCheck[j].Index].ProcessLockingLeft = false;
                    sharedData->SharedRangeLocks[LocksToCheck[j].Index].valid = false;
                }
            }
        }
    }
    cout << "Writer " << getpid() << " has passed all locks\n";
    cout.flush();
    //When out of this loop the process has passed all the locks and has entered the critical section of the shared memory
    t2 = ( double ) times (& tb2) ;     //Measure time waiting at locks
    long seconds =  (t2 - t1) / ticspersec;
    if (sharedData->MaxWaiting < seconds)
        sharedData->MaxWaiting = seconds;
    sharedData->numActriveWriters++;
    cout << "Number of active writers is " << sharedData->numActriveWriters << endl;
    cout.flush();
    t1 = ( double ) times (& tb1);
    sharedData->ActiveWriters[sharedData->numActriveWriters-1] = getpid();
    //The process is going to write to the record, no need to do something else with the shared memory
    
    sem_post(&sharedData->semaphore);    //Unlock the shared memory
    //Write to the file
    MyRecord record;
    int FileDescriptor = open(dataFile, O_RDWR);    //Open the file
    if (FileDescriptor == -1){
        cout << "open failed" << endl;
        return ERROR;
    }
    pread(FileDescriptor, &record, sizeof(MyRecord), recid*sizeof(MyRecord));   //Read the record
    record.balance += atoi(valueString);        //Change the balance
    pwrite(FileDescriptor, &record, sizeof(MyRecord), recid*sizeof(MyRecord)); //Write the record
    int TimeToSleep = rand()%atoi(timeString);      //Sleep for a random amount of time
    usleep(TimeToSleep);
    //cout << "Changed record " << recid << "\n";
    cout << "New Record is " << record.custid << " " << record.LastName << " " << record.FirstName << " " << record.balance << "\n";
    cout.flush();
    close(FileDescriptor);      //Close the file
    sem_wait(&sharedData->semaphore);    //Lock the shared memory
    for (int j = 0 ; j < sharedData->SharedRangeLocks[LockPosition].NumberOfBlocked ; j++){   //Post the semaphore of the lock the correct number of times
        sem_post(&sharedData->SharedRangeLocks[LockPosition].semaphore);
    }
    sharedData->SharedRangeLocks[LockPosition].ProcessLockingLeft = true;   // When process terminates it will set this to true
    // The process becomes invalid only when no other process is waiting at the lock
    //Remove process from active writers
    for (int j = 0 ; j <sharedData->numActriveWriters  ; j++){
        //Find the process in the array of active writers
        if (sharedData->ActiveWriters[j] == getpid()){
            for (int k = j ; k < sharedData->numActriveWriters-1 ; k++){
                //Shift the array to the left
                sharedData->ActiveWriters[k] = sharedData->ActiveWriters[k+1];
            }
            break;
        }
    }
    sharedData->numActriveWriters--;    //Decrease the number of active writers
    t2 = ( double ) times (& tb2) ;     //Measure time waiting at locks
    seconds = (t2 - t1) / ticspersec;   
    sharedData->WritersTime += seconds;    //Add the time to the total time
    sharedData->numWriters++;    //Increase the number of writers that have written to the file
    sharedData->numRecordsProecessed++;     //Increase the number of records processed by 1 since only 1 record was written
    cout << "Writer " << getpid() << " has finished\n";
    cout.flush();
    sem_post(&sharedData->semaphore);       //Unlock the shared memory
}