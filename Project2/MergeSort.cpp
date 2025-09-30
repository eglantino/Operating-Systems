#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "utils.h"
#include <sys/times.h>
using namespace std;

void merge(MyRecord * arr, int left, int mid, int right) {
    int i, j, k;
    int n1 = mid - left + 1;
    int n2 = right - mid;
    
    // Create temporary arrays to hold the left and right sub-arrays
    MyRecord * leftArr = (MyRecord *)malloc(n1 * sizeof(MyRecord));
    MyRecord * rightArr = (MyRecord *)malloc(n2 * sizeof(MyRecord));

    // Copy data to temporary arrays leftArr and rightArr
    for (i = 0; i < n1; i++){
        leftArr[i].custid = arr[left + i].custid;
        strcpy(leftArr[i].postcode, arr[left + i].postcode);
        strcpy(leftArr[i].LastName, arr[left + i].LastName);
        strcpy(leftArr[i].FirstName, arr[left + i].FirstName);
    }
    for (i = 0; i < n2; i++){
        rightArr[i].custid = arr[mid + 1 + i].custid;
        strcpy(rightArr[i].postcode, arr[mid + 1 + i].postcode);
        strcpy(rightArr[i].LastName, arr[mid + 1 + i].LastName);
        strcpy(rightArr[i].FirstName, arr[mid + 1 + i].FirstName);
    }

    // Merge the temporary arrays back into arr[left..right]
    i = 0;
    j = 0;
    k = left;
    while (i < n1 && j < n2) {
        if (RecordStrCmp(leftArr[i], rightArr[j]) <= 0) {
            arr[k].custid = leftArr[i].custid;
            strcpy(arr[k].postcode, leftArr[i].postcode);
            strcpy(arr[k].LastName, leftArr[i].LastName);
            strcpy(arr[k].FirstName, leftArr[i].FirstName);
            i++;
        } else {
            arr[k].custid = rightArr[j].custid;
            strcpy(arr[k].postcode, rightArr[j].postcode);
            strcpy(arr[k].LastName, rightArr[j].LastName);
            strcpy(arr[k].FirstName, rightArr[j].FirstName);
            j++;
        }
        k++;
    }

    // Copy the remaining elements of leftArr and rightArr, if any
    while (i < n1) {
        arr[k].custid = leftArr[i].custid;
        strcpy(arr[k].postcode, leftArr[i].postcode);
        strcpy(arr[k].LastName, leftArr[i].LastName);
        strcpy(arr[k].FirstName, leftArr[i].FirstName);
        i++;
        k++;
    }
    while (j < n2) {
        arr[k].custid = rightArr[j].custid;
        strcpy(arr[k].postcode, rightArr[j].postcode);
        strcpy(arr[k].LastName, rightArr[j].LastName);
        strcpy(arr[k].FirstName, rightArr[j].FirstName);
        j++;
        k++;
    }
    free(leftArr);
    free(rightArr);
    // Free the memory allocated for temporary arrays
}

void mergeSort(MyRecord * arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;

        // Sort first and second halves
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);

        // Merge the sorted halves
        merge(arr, left, mid, right);
    }
}


int main(int argc, char * argv[]) {
   
    double t1 , t2 , cpu_time ;
    struct tms tb1 , tb2 ;
    double ticspersec ;
    ticspersec = ( double ) sysconf ( _SC_CLK_TCK );
    t1 = ( double ) times (& tb1) ;

    int FileDescriptor,FirstRecord,LastRecord,DataSize,i,pid;
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
        mergeSort(data, 0, DataSize-1);
        
        for (i = 0; i < DataSize; i++){
            write(STDOUT_FILENO,&(data[i]),sizeof(MyRecord));
        }
        delete []data;
    }
    kill(pid, SIGUSR2);
    MyRecord rec;

    //Write also the special record with the times
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