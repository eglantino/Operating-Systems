//This is the file in which mysort exe is based on, it is the root file

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include "utils.h"
#define SIZEofBUFF 20
#define SSizeofBUFF 6
#define ERROR -1
using namespace std;

int Sig1Counter = 0 , Sig2Counter = 0;
void signalHandler(int signal) {            
    //Handler for signal each time we increase the value of the according variable by one
    if (signal == SIGUSR1) 
        Sig1Counter++;
    else if (signal == SIGUSR2)
        Sig2Counter++;
}
int main(int argc, char * argv[]){
    if (argc != 9){     //Since our program only accpets calls as described in the assignment
        cout << "Wrong number of arguments!\n";
        return ERROR;
    }
    
    char * dataFile=NULL, *kOfChild = NULL, *e1 = NULL, *e2 = NULL; 
    for (int i = 1 ; i < argc ; i +=2){
        //Simple reading of the arguments
        if(strcmp(argv[i],"-i") == 0 ){
            if ( ( dataFile = (char*)malloc(sizeof(char)*strlen(argv[i+1])+1) ) ==NULL){
                cout << "Could not allocate memory\n";
                return ERROR;
            }
            strcpy(dataFile,argv[i+1]);
        }
        else if(strcmp(argv[i],"-k") == 0 ){
            if ( ( kOfChild = (char*)malloc(sizeof(char)*strlen(argv[i+1])+1) ) ==NULL){
                cout << "Could not allocate memory\n";
                return ERROR;
            }
            strcpy(kOfChild,argv[i+1]);
        }
        else if(strcmp(argv[i],"-e1") == 0 ){
            if ( ( e1 = (char*)malloc(sizeof(char)*strlen(argv[i+1])+1) ) ==NULL){
                cout << "Could not allocate memory\n";
                return ERROR;
            }
            strcpy(e1,argv[i+1]);
        }
        else if(strcmp(argv[i],"-e2") == 0 ){
            if ( ( e2 = (char*)malloc(sizeof(char)*strlen(argv[i+1])+1) ) ==NULL){
                cout << "Could not allocate memory\n";
                return ERROR;
            }
            strcpy(e2,argv[i+1]);
        }
        else{
            //Invaliud flag
            cout << "Invalid input as argument\n";
            return ERROR;
        }
    }
    if (dataFile == NULL || kOfChild == NULL || e1 == NULL || e2 == NULL){
        //Means that at least one of the flags was absent
        cout << "Wrong usage of the program, the usage is \n";
        cout << "./mysort -i DataFile -k NumofChildren -e1 sorting1 -e2 sorting2\n";
        return ERROR;
    }
    for (int i = 0; i < (int)strlen(kOfChild); i++) {
        // Not an integer check
        if(!isdigit(kOfChild[i])){
            cout<< "Malformed Input\n";
            return ERROR;
        }
    }
    int k ;
    k = atoi(kOfChild);
    if ( k <= 0 ){
        cout << "K should be a positive integer\n";
        return ERROR;
    }
    signal(SIGUSR1, signalHandler);
    signal(SIGUSR2, signalHandler);
    int **pipe_to_child = new int *[k];
    if (! pipe_to_child){
        cout << "Memory allocation failed\n";
        return ERROR;
    }
    int **pipe_from_child = new int *[k];
    if (! pipe_from_child){
        cout << "Memory allocation failed\n";
        return ERROR;
    }
    //For each child we have one pipe from and one to the parent
    pid_t child_pid;
    int numOfrecords;
    MyRecord rec;
    int FileDescriptor;
    long lSize;
    FileDescriptor  =  open(dataFile,O_RDONLY);     //Open file form the arguments
    if (FileDescriptor == -1){
        cout << "Opening file " << dataFile <<" failed\n";
        return ERROR;
    }
    int RecordsPerMerger;
    lSize = lseek (FileDescriptor , 0 , SEEK_END);
    lseek (FileDescriptor , 0 , SEEK_SET);
    numOfrecords = (int) lSize/sizeof(rec);
    RecordsPerMerger = numOfrecords / k;
    //Split the number of records to the mergers
    for (int i = 0 ; i < k ; i++){
        //Memory allcoation for pipes
        pipe_to_child[i] = new int[2];
        if (! pipe_to_child[i]){
            cout << "Memory allocation failed\n";
            return ERROR;
         }
        pipe_from_child[i] = new int[2];
        if (! pipe_from_child[i]){
            cout << "Memory allocation failed\n";
            return ERROR;
        }
    }
    for (int i = 0 ; i < k ; i ++){
        if (pipe(pipe_to_child[i]) == -1) {
            perror("pipe");
            exit(1);
        }
        if (pipe(pipe_from_child[i]) == -1) {
            perror("pipe");
            exit(1);
        }
        child_pid = fork();
        if (child_pid == -1) {
            perror("fork");
            exit(1);
        }
        if (child_pid == 0){
            //Child process, close the unused pipes
            close(pipe_to_child[i][1]);
            close(pipe_from_child[i][0]);
            dup2(pipe_to_child[i][0], STDIN_FILENO);        //Redirect stdin of the child
            dup2(pipe_from_child[i][1], STDOUT_FILENO);     //Redirect stdout of the child
            char exe[20];
            strcpy(exe,"./Merger");
            char* const Args[] = {exe,e1,e2,NULL};  
            //List consists of the executable for merge sort and quicksort
            execvp(Args[0], Args);
            perror("Exec failed");
            exit(1);
        }
    }
    
    char int_str[SIZEofBUFF];
    for (int i = 0 ; i < k; i ++){
        //Paret process
        close(pipe_to_child[i][0]);     //Close unused pipes
        close(pipe_from_child[i][1]);
        sprintf(int_str, "%d\n", getpid()); 
        write(pipe_to_child[i][1],int_str,strlen(int_str));    // Write parent's pid to parent
        sprintf(int_str, "%d\n", FileDescriptor);           //Write file descriptor to pipe
        write(pipe_to_child[i][1],int_str,strlen(int_str));         
        sprintf(int_str, "%d\n", k-i);                      
        //Write k-i in pipe, which represent how many sorters will the merger create
        write(pipe_to_child[i][1],int_str,strlen(int_str));
        sprintf(int_str,"%d\n",i*RecordsPerMerger);     //The index of the first record to be sorted
        write(pipe_to_child[i][1],int_str,strlen(int_str));     //Write it to pipe
        if ( i == k-1)          //Last merger takes the possible remaining records
            sprintf(int_str,"%d\n",i*RecordsPerMerger+RecordsPerMerger+numOfrecords%k);     //The index of the last record to be sorted
        else
            sprintf(int_str,"%d\n",i*RecordsPerMerger+RecordsPerMerger);
        write(pipe_to_child[i][1],int_str,strlen(int_str));     //Write the last index
    }

    /*
    MyRecord ** ReturnedRecords = new MyRecord*[k];
    int * Indexes = new int[k];
    //For each of the pip
    if (!ReturnedRecords || !Indexes){
        cout << "Memorey allocation failed\n";
        return ERROR;
    }
    for (int i =0 ; i < k; i++){
        Indexes[i] = 0;
        if (i == k-1)
            ReturnedRecords[i] = new MyRecord[RecordsPerMerger+numOfrecords%k +1];
        else
            ReturnedRecords[i] = new MyRecord[RecordsPerMerger+1];
        if (!ReturnedRecords[i]){
            cout << "Memorey allocation failed\n";
            return ERROR;
        }
    }
    int j;
    for (int i = 0 ; i < k ; i ++){
        j=0;
        do{
            read(pipe_from_child[i][0],&ReturnedRecords[i][j],sizeof(rec));
        }while(ReturnedRecords[i][j++].custid != -666);
    }
    for (int i = 0 ; i < numOfrecords ; i ++){
        int Position = -1;
        for (int x = 0 ; x < k ;x++){
            
            if (ReturnedRecords[x][Indexes[x]].custid == -666)
                continue;
            if (Position == -1 ){
                Position = x;
                continue;
            }
            if ( RecordStrCmp(ReturnedRecords[x][Indexes[x]],ReturnedRecords[Position][Indexes[Position]]) < 0)
                Position = x;
        }
        printf("%d %s %s %s \n", \
		ReturnedRecords[Position][Indexes[Position]].custid, ReturnedRecords[Position][Indexes[Position]].LastName, \
                ReturnedRecords[Position][Indexes[Position]].FirstName, ReturnedRecords[Position][Indexes[Position]].postcode);
        ++(Indexes[Position]);
    }
    int leaf=0;
    for (int i = 0 ; i < k ; i++){
        for (int j = 0 ; j < k-i; j++){
            printf("For the %d leaf, run time was %s and cpu %s\n",leaf++,ReturnedRecords[i][Indexes[i]].FirstName,ReturnedRecords[i][Indexes[i]].LastName);
            read(pipe_from_child[i][0],&ReturnedRecords[i][Indexes[i]],sizeof(rec));
        }
    }

    */





    MyRecord * ReturnedRecords = new MyRecord[k];
    //For each of the chidren hold one record
    if (!ReturnedRecords){
        cout << "Memorey allocation failed\n";
        return ERROR;
    }
    
    for (int i = 0 ; i < k ; i ++)
        //Read the first result fromt he pipe that corresponds to i-th child  
        read(pipe_from_child[i][0],&ReturnedRecords[i],sizeof(rec));
    for (int i = 0 ; i < numOfrecords ; i ++){
        // We will displau numOfrecords results
        int Position = -1;
        //Position corresponds to the index of th pipe which is consumed
        //Position == - 1 means that for the i-th result nothing is consumed yet from the children
        for (int x = 0 ; x < k ;x++){
            // For loop that finds the "biggest" of all results
            if (ReturnedRecords[x].custid == -666)
                //End of the results for x child
                continue;
            if (Position == -1 ){
                //First result we got, hold it
                Position = x;
                continue;
            }
            //If we are here, there are more results and something has already been consumed, so Position != _1
            if ( RecordStrCmp(ReturnedRecords[x],ReturnedRecords[Position]) < 0)
                Position = x;
        }
        //If we are here, we know that i-th "biggest" result, is the ont that comes from Position pipe
        printf("%d %s %s %s \n", \
		ReturnedRecords[Position].custid, ReturnedRecords[Position].LastName, \
                ReturnedRecords[Position].FirstName, ReturnedRecords[Position].postcode);
        int Bytes = read(pipe_from_child[Position][0],&ReturnedRecords[Position],sizeof(rec));
        //Since we consumed a result from Position pipe, we have to read the next result of that pipe
        if (Bytes < sizeof(rec))
            ReturnedRecords[Position].custid = -1;
    }
    int leaf=0;
    for (int i = 0 ; i < k ; i++){
        //Print leaf times
        for (int j = 0 ; j < k-i; j++){
            printf("For the %d leaf, run time was %s and cpu %s\n",leaf++,ReturnedRecords[i].FirstName,ReturnedRecords[i].LastName);
            read(pipe_from_child[i][0],&ReturnedRecords[i],sizeof(rec));
        }
    }

    for (int i = 0; i < k; i++) {
        //Wait for children
        int status;
        pid_t child_pid = wait(&status);
    }
    printf("received %d SIG1 and %d SIG2\n",Sig1Counter,Sig2Counter);
    close(FileDescriptor);
    free(dataFile);
    free(e1);
    free(e2);
    for (int i = 0 ; i < k ; i ++){
        close(pipe_from_child[i][0]);
        delete [] pipe_from_child[i];
    }
    delete [] pipe_from_child;
    for (int i = 0 ; i < k ; i ++){
        close(pipe_to_child[i][1]);
        delete [] pipe_to_child[i];
    }
    delete [] pipe_to_child;
    delete []ReturnedRecords;
    free(kOfChild);
}