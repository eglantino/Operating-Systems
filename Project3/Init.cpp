#include <iostream>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/wait.h>
#include "Shared.h"
#include <signal.h>

#include <cstring>    // For memset
#define DEFAULT_READERS 10
#define DEFAULT_WRITERS 10
#define DEFAULT_MAX_VALUE 100
#define DEFAULT_TIME 20000000
using namespace std;


int main(int argc, char * argv[]){
    char * readers = NULL, * writers = NULL, *fileName = NULL, * maxValChar=NULL, *WaitTimeChar =NULL; // Arguments for number of readers and writers
    int numReaders = DEFAULT_READERS, numWriters = DEFAULT_WRITERS, MaxVal =DEFAULT_MAX_VALUE,WaitTIme=DEFAULT_TIME; // Default values
    
    bool printRecords = false;
    for (int i = 1 ; i < argc ; i +=2){ // Read arguments
        if(strcmp(argv[i],"-r") == 0 ){ // Number of readers
            if ( ( readers = (char*)malloc(sizeof(char)*strlen(argv[i+1])+1) ) ==NULL){
                cout << "Could not allocate memory\n";
                return ERROR;
            }
            strcpy(readers,argv[i+1]);
            numReaders = atoi(readers); // Convert to integer
        }
        else if(strcmp(argv[i],"-w") == 0 ){ // Number of writers
            if ( ( writers = (char*)malloc(sizeof(char)*strlen(argv[i+1])+1) ) ==NULL){
                cout << "Could not allocate memory\n";
                return ERROR;
            }
            strcpy(writers,argv[i+1]);
            numWriters = atoi(writers); // Convert to integer
        }
        else if(strcmp(argv[i],"-v") == 0 ){ // Max value
            if ( ( maxValChar = (char*)malloc(sizeof(char)*strlen(argv[i+1])+1) ) ==NULL){
                cout << "Could not allocate memory\n";
                return ERROR;
            }
            strcpy(maxValChar,argv[i+1]);
            MaxVal = atoi(maxValChar); // Convert to integer
        }
        else if(strcmp(argv[i],"-f") == 0 ){ // File name
            if ( ( fileName = (char*)malloc(sizeof(char)*strlen(argv[i+1])+1) ) ==NULL){
                cout << "Could not allocate memory\n";
                return ERROR;
            }
            strcpy(fileName,argv[i+1]);
        }
        else if (strcmp(argv[i],"-d") == 0 ){ // Wait time
            if ( ( WaitTimeChar = (char*)malloc(sizeof(char)*strlen(argv[i+1])+1) ) ==NULL){
                cout << "Could not allocate memory\n";
                return ERROR;
            }
            strcpy(WaitTimeChar,argv[i+1]);
            WaitTIme = atoi(WaitTimeChar); // Convert to integer
        }
        else if(strcmp(argv[i],"-print") == 0 ){ // Print records
            printRecords = true;    // Set flag
            i--;    // Decrease i to read next argument, since this argument does not have a value
        }
        else{
            cout << "Invalid input as argument\n";
            return ERROR;
        }
    }
    if (readers != NULL){   // Check if readers argument was given
        if (numReaders == 0){   // Check if number of readers is valid
            cout << "Invalid number of readers, default will be used\n";
            numReaders = DEFAULT_READERS;
        }
        free(readers);
    }
    if (writers != NULL){   // Check if writers argument was given
        if (numWriters == 0){  // Check if number of writers is valid
            cout << "Invalid number of writers, default will be used\n";
            numWriters = DEFAULT_WRITERS;
        }
        free(writers);
    }
    if (fileName == NULL){  // Check if file name was given
        cout << "No file name given\n";
        return ERROR;
    }
    if (maxValChar != NULL){    // Check if max value argument was given
        if (MaxVal == 0){   // Check if max value is valid
            cout << "Invalid max value, default will be used\n";
            MaxVal = DEFAULT_MAX_VALUE;
        }
        free(maxValChar);
    }
    if (WaitTimeChar != NULL){  // Check if wait time argument was given
        if (WaitTIme == 0){ // Check if wait time is valid
            cout << "Invalid wait time, default will be used\n";
            WaitTIme = DEFAULT_TIME;
        }
        free(WaitTimeChar);
    }
    const char *shmName = "/SharedMemory";  // Name of shared memory
    cout << "Creating shared memory with name: " << shmName << endl;    // Print name of shared memory
    int shm_fd = shm_open(shmName, O_CREAT | O_RDWR | O_EXCL, 0666);    // Create shared memory
    if (shm_fd == -1){  // Check if shared memory was created
        if (errno == EEXIST) { // Shared memory already exists
            shm_unlink(shmName);   // Unlink existing shared memory
            shm_fd = shm_open(shmName, O_CREAT | O_RDWR, 0666);     // Create shared memory
            if (shm_fd == -1) {     // Check if shared memory was created
                cout << "shm_open failed" << endl;
                return ERROR;
            }
        } else {
            cout << "shm_open failed" << endl;
            return ERROR;
        }
    }
    int size = sizeof(SharedData);      // Size of shared memory
    if (ftruncate(shm_fd, size) == -1){     // Truncate shared memory
        cout << "ftruncate failed" << std::endl;
        return ERROR;
    }
    SharedData * sharedData = (SharedData *)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); // Map shared memory
    
    if (sharedData == MAP_FAILED){
        cout << "mmap failed" << std::endl;
        return ERROR;
    }
    memset(sharedData, 0, sizeof(SharedData)); // Initialize shared memory
    sharedData->numActiveReaders = 0; 
    sharedData->numActriveWriters = 0;
    sharedData->numRecordsProecessed = 0;
    sharedData->numReaders = 0;
    sharedData->numWriters = 0;
    sharedData->Readersime = 0;
    sharedData->WritersTime = 0;
    sharedData->MaxWaiting = 0;
    sem_init(&sharedData->semaphore, 1, 1);
    for (int i = 0 ; i < MAX_PROCESSES; i++){
        //Initialize range locks
        sharedData->SharedRangeLocks[i].valid = false;
        sharedData->SharedRangeLocks[i].ProcessLockingLeft = false;
        sharedData->SharedRangeLocks[i].timesChanged=0;
    }
    int pid;
    MyRecord record;
    //Print first 10 records of the file    
    long lSize;
    int FileDescriptor = open(fileName, O_RDONLY);
    if (FileDescriptor == -1){
        cout << "open failed" << endl;
        return ERROR;
    }
    lSize = lseek (FileDescriptor , 0 , SEEK_END);
    lseek (FileDescriptor , 0 , SEEK_SET);
    int numOfrecords = (int) lSize/sizeof(record);
    if (printRecords){
        for (int i = 0; i < numOfrecords ;i++){
            if (read(FileDescriptor, &record, sizeof(MyRecord)) == -1){
                cout << "read failed" << endl;
                return ERROR;
            }
            cout << record.custid << " " << record.LastName << " " << record.FirstName << " " << record.balance << endl;
        }
    }   
    close(FileDescriptor);
    for (int i = 0 ; i < numReaders ; i++){
        int r1 = rand()%numOfrecords;
        int r2 = r1+rand()%(numOfrecords-r1);
        pid = fork();
        if (pid == 0 ){
            char int_str[SIZEofBUFF],int_str2[SIZEofBUFF];
            sprintf(int_str, "%d,%d",r1,r2);
            sprintf(int_str2, "%d",WaitTIme);
            if (execl("./reader", "./reader", "-f", fileName,"-l", int_str, "-d",int_str2, "-s", shmName, NULL)==-1){
                perror("execl");
            }
        }
    }
    for (int i = 0 ; i < numWriters ; i++){
        int r1 = rand()%numOfrecords;
        int r2 = rand()%(2*MaxVal)-MaxVal;
        pid = fork();
        if (pid == 0){
            char int_str[SIZEofBUFF],int_str2[SIZEofBUFF],int_str3[SIZEofBUFF];
            sprintf(int_str, "%d",r1);
            sprintf(int_str2, "%d",WaitTIme);
            sprintf(int_str3, "%d",r2);
            if ( execl("./writer", "./writer", "-f", fileName,"-l", int_str, "-d", int_str2 , "-s", shmName,"-v",int_str3, NULL) == -1 ){
                perror("execl");
            }
        }
    }
    free(fileName);
    
    /*
    numWriters = 5;
    numReaders = 5;
    for(int i = 0 ; i < numWriters + numReaders ; i++){
        int r2 = rand()%(2*MaxVal)-MaxVal;
        pid = fork();
        if (pid == 0 && i % 2 == 0){
            char int_str[SIZEofBUFF],int_str2[SIZEofBUFF],int_str3[SIZEofBUFF];
            sprintf(int_str, "%d",r2);
            if ( execl("./writer", "./writer", "-f", fileName,"-l", "3", "-d", "200000000" , "-s", shmName,"-v",int_str, NULL) == -1 ){
                perror("execl");
            }
        }
        else if (pid == 0 && i % 2 == 1){
            char int_str[SIZEofBUFF],int_str2[SIZEofBUFF];
            sprintf(int_str, "%d,%d",0,4);
            if (execl("./reader", "./reader", "-f", fileName,"-l", int_str, "-d","20000000", "-s", shmName, NULL)==-1){
                perror("execl");
            }
        }
    }
    */
    // Wait for all children to finish
    for (int i = 0 ; i < numWriters+numReaders ; i++){
        wait(NULL);
    }
    //Print shared statistics
    cout << "Number of readers: " << sharedData->numReaders << endl;
    cout << "Mean reader time: " << sharedData->Readersime/sharedData->numReaders << endl;
    cout << "Number of writers: " << sharedData->numWriters << endl;
    cout << "Mean writer time: " << sharedData->WritersTime/sharedData->numWriters << endl;
    cout << "Max waiting time: " << sharedData->MaxWaiting << endl;
    cout << "Number of records processed: " << sharedData->numRecordsProecessed << endl;
    
    // Unmap shared memory
    if (munmap(sharedData, size) == -1){
        cout << "munmap failed" << std::endl;
        return ERROR;
    }
    // Close shared memory
    if (close(shm_fd) == -1){
        cout << "close failed" << std::endl;
        return ERROR;
    }
    // Unlink shared memory
    if (shm_unlink("/SharedMemory") == -1){
        cout << "shm_unlink failed" << std::endl;
        return ERROR;
    }
    
    
    return 0;
}