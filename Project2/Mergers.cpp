//Mergers file, the inner node

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
using namespace std;
#include "utils.h"

int main(int argc, char * argv[]){
    //This code is very simillar to the coordinator.cpp
    int FileDescriptor,NumberOfChilds,FirstRecord,LastRecord,DataSize,pid;
    cin >> pid;
    cin >> FileDescriptor;
    cin >> NumberOfChilds;
    cin >> FirstRecord;
    cin >> LastRecord;
    DataSize = LastRecord - FirstRecord ;
    int **pipe_to_child = new int *[NumberOfChilds];
    if (!pipe_to_child){
        fprintf(stderr,"Memory allocation failed\n");
        return 0;
    }
    int **pipe_from_child = new int *[NumberOfChilds];
    if (!pipe_from_child){
        fprintf(stderr,"Memory allocation failed\n");
        return 0;
    }
    pid_t child_pid;
    for (int i = 0 ; i < NumberOfChilds ; i++){
        pipe_to_child[i] = new int[2];
        if (!pipe_to_child[i]){
            fprintf(stderr,"Memory allocation failed\n");
            return 0;
        }
        pipe_from_child[i] = new int[2];
        if (!pipe_from_child[i]){
            fprintf(stderr,"Memory allocation failed\n");
            return 0;
        }
    }
    char * Method ;
    if (rand()%2)
        //Coin flip for the first method
        Method = argv[1];
    else
        Method = argv[2];
    for (int i = 0 ; i < NumberOfChilds ; i++){
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
        if (child_pid == 0 ){       //Child
            close(pipe_to_child[i][1]);
            close(pipe_from_child[i][0]);
            dup2(pipe_to_child[i][0], STDIN_FILENO);
            dup2(pipe_from_child[i][1], STDOUT_FILENO);
            char exe[20];
            strcpy(exe,"./Sorter");
            char* const Args[] = {exe,Method,NULL};  
            execvp(Args[0], Args);
            perror("Exec failed");
            exit(1);
        }
        else{
            //Chnage the method to the other one
            if (Method == argv[1])
                Method = argv[2];
            else
                Method = argv[1];
        }
    }
    int RecordsPerSorter = DataSize/NumberOfChilds;
        for (int i = 0 ; i < NumberOfChilds  ; i++){       //Parent

            close(pipe_to_child[i][0]);
            close(pipe_from_child[i][1]);
            int RecordsPerSorter = DataSize/NumberOfChilds;
            char int_str[20];
            sprintf(int_str, "%d\n", pid); 
            write(pipe_to_child[i][1],int_str,strlen(int_str));    // Write parent's pid to parent
            sprintf(int_str, "%d\n", FileDescriptor);           //Write file desxriptor to pipe
            write(pipe_to_child[i][1],int_str,strlen(int_str));         
            sprintf(int_str,"%d\n",FirstRecord+ i*RecordsPerSorter);     //The index of the first record to be sorted
            write(pipe_to_child[i][1],int_str,strlen(int_str));     //Write it to pipe
            if ( i == NumberOfChilds-1)          //Last merger takes the possible remainings
                sprintf(int_str,"%d\n",LastRecord);     //The index of the last record to be sorted
            else
                sprintf(int_str,"%d\n",FirstRecord+i*RecordsPerSorter+RecordsPerSorter);
            write(pipe_to_child[i][1],int_str,strlen(int_str));     //Write the last index
            close(pipe_to_child[i][1]);
        }
     /*
    MyRecord ** ReturnedRecords = new MyRecord*[NumberOfChilds];
     int * Indexes = new int[NumberOfChilds];
    //For each of the pip
    for (int i =0 ; i < NumberOfChilds; i++){
        Indexes[i] = 0;
        if (i == NumberOfChilds-1)
            ReturnedRecords[i] = new MyRecord[RecordsPerSorter+RecordsPerSorter%NumberOfChilds + 1];
        else
            ReturnedRecords[i] = new MyRecord[RecordsPerSorter + 1];
        if (!ReturnedRecords[i]){
            cout << "Memorey allocation failed\n";
            return ERROR;
        }
    }
    int j;
    for (int i = 0 ; i < NumberOfChilds ; i ++){
        j=0;
        do{
            read(pipe_from_child[i][0],&ReturnedRecords[i][j],sizeof(MyRecord));
        }while(ReturnedRecords[i][j++].custid != -666);
    }
    for (int i = 0 ; i < DataSize ; i ++){
        int Position = -1;
        for (int x = 0 ; x < NumberOfChilds ;x++){
            if (ReturnedRecords[x][Indexes[x]].custid == -666)
                continue;
            
            if (Position == -1 ){
                Position = x;
                continue;
            }
            if ( RecordStrCmp(ReturnedRecords[x][Indexes[x]],ReturnedRecords[Position][Indexes[Position]]) < 0)
                Position = x;
        }
        write(STDOUT_FILENO,&(ReturnedRecords[Position][Indexes[Position]]),sizeof(MyRecord));
        ++(Indexes[Position]);
    }

    for (int i = 0 ; i < NumberOfChilds; i++)
        write(STDOUT_FILENO,&(ReturnedRecords[i][Indexes[i]]),sizeof(MyRecord));

     */






    

    MyRecord * ReturnedRecords = new MyRecord[NumberOfChilds];
    if (!ReturnedRecords){
        fprintf(stderr,"Memory allocation faile\n");
        return 0;
    }
    char buff[20];
    for (int i = 0 ; i < NumberOfChilds ; i ++)
        read(pipe_from_child[i][0],&ReturnedRecords[i],sizeof(MyRecord));
    

    for (int i = 0 ; i < DataSize ; i ++){
        int Position = -1;
        for (int x = 0 ; x < NumberOfChilds ;x++){
            if (ReturnedRecords[x].custid == -666)
                continue;
            
            if (Position == -1 ){
                Position = x;
                continue;
            }
            if ( RecordStrCmp(ReturnedRecords[x],ReturnedRecords[Position]) < 0)
                Position = x;
        }
        write(STDOUT_FILENO,&(ReturnedRecords[Position]),sizeof(MyRecord));
        fsync(STDOUT_FILENO);
        int Bytes = read(pipe_from_child[Position][0],&ReturnedRecords[Position],sizeof(MyRecord));
        if (Bytes < sizeof(MyRecord))
            ReturnedRecords[Position].custid = -1;
    }

    for (int i = 0 ; i < NumberOfChilds; i++)
        //Return the sorted list to the pipe from which parent reads
        write(STDOUT_FILENO,&(ReturnedRecords[i]),sizeof(MyRecord));
    
    for (int i = 0; i < NumberOfChilds; i++) {
        int status;
        pid_t child_pid = wait(&status);
    }
    for (int i = 0 ; i < NumberOfChilds ; i ++){
        close(pipe_from_child[i][0]);
        delete [] pipe_from_child[i];
    }
    delete [] pipe_from_child;
    for (int i = 0 ; i < NumberOfChilds ; i ++){
        close(pipe_to_child[i][1]);
        delete [] pipe_to_child[i];
    }
    delete [] pipe_to_child;
    delete []ReturnedRecords;
    kill(pid, SIGUSR1);       //Send SIGUSR1
    return 0;   
}