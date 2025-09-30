#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include "Compare.h"
#include <string.h>
#define ERROR -1
using namespace std;

int main (int argc, char *argv[]) {
    if (argc != 6 && argc != 4){
        cout << "Usage: cmpcats [-d] path1 path2 [-s] destination" << endl;
        return ERROR;
    }
    char *path1=NULL, *path2=NULL, *destination=NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            if ( (path1 = (char*)malloc(strlen(argv[i+1])+1)) == NULL) {
                cout << "Error allocating memory" << endl;
                return ERROR;
            }
            if ( (path2 = (char*)malloc(strlen(argv[i+2])+1)) == NULL) {
                cout << "Error allocating memory" << endl;
                return ERROR;
            }
            strcpy(path1, argv[i+1]);
            strcpy(path2, argv[i+2]);
            i+=2;
        }
        else if (strcmp(argv[i], "-s") == 0) {
            if ( (destination = (char*)malloc(strlen(argv[i+1])+1)) == NULL) {
                cout << "Error allocating memory" << endl;
                return ERROR;
            }
            strcpy(destination, argv[i+1]);
            i++;
        }
        else {
            cout << "Usage: cmpcats [-d] path1 path2 [-s] destination" << endl;
            return ERROR;
        }
    }
    if (destination == NULL) {
        Compare(path1, path2);
    }
    else {
        MergeFiles(path1, path2, destination);
    }
    
    return 0;
}