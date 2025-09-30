#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "utils.h"
#include <fstream>
#include <fcntl.h>
#include <poll.h> 
#include <signal.h>
#include <sstream>
#include <sys/times.h>

using namespace std;

int main(int argc, char * argv[]) {
    
    int i, sum = 0;
    int FileDescriptor,FirstRecord,LastRecord,pid;
    cin >> pid;
    cin >> FileDescriptor;
    cin >> FirstRecord;
    cin >> LastRecord;
    // In the child process, use exec to run the MergeSort program
    
    char int_str[20],int_str2[20],int_str3[20],int_str4[20];
    sprintf(int_str, "%d", pid);
    sprintf(int_str2, "%d", FileDescriptor); 
    sprintf(int_str3, "%d", FirstRecord); 
    sprintf(int_str4, "%d", LastRecord); 
    char* const Args[] = {argv[1],int_str,int_str2,int_str3,int_str4,NULL};

    execvp(Args[0], Args);
    perror("Exec failed");
    exit(1);
    return 0;
}