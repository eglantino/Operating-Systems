#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
using namespace std;
int main() {
    struct stat fileStat;
    if(stat("dirA", &fileStat) < 0)    
        return 1;

    printf("Information for example.txt:\n");
    printf("---------------------------\n");
    printf("File Size: \t\t%ld bytes\n",fileStat.st_size);
    printf("Number of Links: \t%ld\n",fileStat.st_nlink);
    printf("File inode: \t\t%ld\n",fileStat.st_ino);
    printf("It is a: \t\t");
    if(S_ISREG(fileStat.st_mode))
        printf("regular file\n");
    else if(S_ISDIR(fileStat.st_mode))
        printf("directory\n");
    else if(S_ISCHR(fileStat.st_mode))
        printf("character device\n");
    else if(S_ISBLK(fileStat.st_mode))
        printf("block device\n");
    else if(S_ISFIFO(fileStat.st_mode))
        printf("FIFO\n");
    else if(S_ISLNK(fileStat.st_mode))
        printf("symbolic link\n");
    else if(S_ISSOCK(fileStat.st_mode))
        printf("socket\n");
    else
        printf("unknown\n");

    //Print all members of stat structure

    cout << fileStat.st_blocks << endl;
    cout << fileStat.st_blksize << endl;
    cout << fileStat.st_dev << endl;
    cout << fileStat.st_gid << endl;
    cout << fileStat.st_mode << endl;
    cout << fileStat.st_rdev << endl;
    cout << fileStat.st_uid << endl;
    cout << fileStat.st_atime << endl;
    cout << fileStat.st_ctime << endl;
    cout << fileStat.st_mtime << endl;
    cout << fileStat.st_size << endl;
    cout << fileStat.st_nlink << endl;
    cout << fileStat.st_ino << endl;

    return 0;
}
