#include <stdio.h>
#include <string.h>
#include <iostream>
#include "utils.h"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/times.h>

using namespace std;

void swap(MyRecord * A, MyRecord * B) {
    int t = A->custid;
    A->custid = B->custid;
    B->custid = t;
    char temp[20];
    strcpy(temp,A->FirstName);
    strcpy(A->FirstName,B->FirstName);
    strcpy(B->FirstName,temp);
    
    strcpy(temp,A->LastName);
    strcpy(A->LastName,B->LastName);
    strcpy(B->LastName,temp);
    strcpy(temp,A->postcode);
    strcpy(A->postcode,B->postcode);
    strcpy(B->postcode,temp);
}

int partition(MyRecord * arr ,int low, int high) {
    
    MyRecord pivot = arr[high];
    pivot.custid=arr[high].custid;
    strcpy(pivot.FirstName,arr[high].FirstName);
    strcpy(pivot.LastName,arr[high].LastName);
    strcpy(pivot.postcode,arr[high].postcode);
    int i = (low - 1);

    for (int j = low; j < high; j++) {
        if (RecordStrCmp(arr[j], pivot) < 0) {
            i++;
            
            swap(&arr[i], &arr[j]);
            
        }
    }
    
    swap(&arr[i + 1], &arr[high]);
    
    return (i + 1);
}

void quickSort(MyRecord * arr, int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);

        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

int main(int argc, char * argv[]) {
    
    double t1 , t2 , cpu_time ;
    struct tms tb1 , tb2 ;
    double ticspersec ;
    int FileDescriptor,FirstRecord,LastRecord,DataSize,i,pid;
    
    ticspersec = ( double ) sysconf ( _SC_CLK_TCK );
    t1 = ( double ) times (& tb1) ;

    pid = atoi(argv[1]);
    FileDescriptor = atoi(argv[2]);
    FirstRecord = atoi(argv[3]);
    LastRecord = atoi(argv[4]);
    DataSize = LastRecord-FirstRecord;
    if (DataSize > 0){
        MyRecord *data = new MyRecord[DataSize];
        if (!data){
            fprintf(stderr,"Memory allocation failed\n");
            return 0;
        }
        for (i = 0; i < DataSize; i++)
            pread(FileDescriptor,&data[i],sizeof(MyRecord),(FirstRecord+i)*sizeof(MyRecord));
        quickSort(data, 0, DataSize - 1);
        for (i = 0; i < DataSize; i++) {
            write(STDOUT_FILENO,&(data[i]),sizeof(MyRecord));
            
        }
        delete []data;
    }
    kill(pid, SIGUSR2);
    MyRecord rec;
    rec.custid = -666;  //The last record is not a true record, to recognize we set id = -666
    //In this special record, the FirstName corresponds to Real Time and LastName to CPU time
    strcpy(rec.FirstName,"Finished");
    t2 = ( double ) times (& tb2) ;
    cpu_time = ( double ) (( tb2 . tms_utime + tb2 . tms_stime ) -( tb1 . tms_utime + tb1 . tms_stime ));
    char str[20];
    sprintf(str, "%f", (t2-t1)/ticspersec);
    strcpy(rec.FirstName,str);
    sprintf(str, "%f", cpu_time / ticspersec );
    strcpy(rec.LastName,str);
    write(STDOUT_FILENO, &rec, sizeof(MyRecord));
    
    
    
    return 0;
}