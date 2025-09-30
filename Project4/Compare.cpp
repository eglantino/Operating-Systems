#include "Compare.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#define MAX 16
using namespace std;
typedef struct Data_Returned   
//This struct is used to return the data from the function CheckDirectories
//It contains the paths to the files that are different in the two directories
//The idea is that the function CheckDirectories will return a pointer to this struct
{
    int DataSize;       //The number of paths that are different
    char ** Data;       //The paths that are different
    int MaxData;        //The maximum number of paths that can be stored in Data
    //We will not increase the size of Data by one every time we add a path
    //Instead we will increase it by a factor of 2
    //This is done to avoid calling realloc every time we add a path
    //This is done to improve the performance of the program
}DataReturned;

void printStringsStartingWith(char *array[], int size, char *prefix) {
    // Print the strings in array that start with prefix
    //Prefi is actually the path of the directory
    int prefixLength = strlen(prefix);
    bool first = true;  // Used to print the directory name only once
    for (int i = 0; i < size; ++i) {
        if (strncmp(array[i], prefix, prefixLength) == 0) { // If the string starts with prefix
            if (array[i][prefixLength] != '/') // If the string is not a subdirectory of prefix
                continue;
            if (first) {
                // Print the directory name
                first = false;
                printf("In %s\n", prefix);
            }
            // Print the string excluding the prefix
            printf("%s\n", array[i] + prefixLength+1);      //+1 to remove the slash
        }
    }
    if (!first)
        cout << endl;
}

void TraverseDirectory(const char *path, DataReturned * ToReturn) {
    //This function is used to find the paths of all the files in a directory
    //The paths are stored in the struct DataReturned
    //The function is called recursively
    //The ToReturn struct is passed as an argument to the function
    //The function will add the paths of the files in the directory to the struct
    DIR *dir = opendir(path);
    struct dirent *entry;
    struct stat statbuf;
    char fullPath[1024];

    if (dir == NULL) {
        perror("Error opening directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        //For every file in the directory
        // Skip the "." and ".." entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Construct full path
        //The full path is the path of the directory + the name of the file
        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);
        stat(fullPath, &statbuf);


        // Check if it's a directory
        if (S_ISDIR(statbuf.st_mode)) {
            ToReturn->DataSize++;
            if (ToReturn->DataSize > ToReturn->MaxData){        //If the size of Data is bigger than MaxData
                //Increase the size of Data by a factor of 2
                ToReturn->MaxData = ToReturn->MaxData * 2;
                ToReturn->Data = (char **) realloc(ToReturn->Data, ToReturn->MaxData * sizeof(char *));
            }
            ToReturn->Data[ToReturn->DataSize - 1] = new char[sizeof(fullPath)];
            //Copy the path of the directory to the Data array
            strcpy(ToReturn->Data[ToReturn->DataSize - 1], fullPath);
            // Recursively call TraverseDirectory
            TraverseDirectory(fullPath,ToReturn);
        } else {
            //Same but for files
            //No need to call TraverseDirectory
            ToReturn->DataSize++;
            if (ToReturn->DataSize > ToReturn->MaxData){
                ToReturn->MaxData = ToReturn->MaxData * 2;
                ToReturn->Data = (char **) realloc(ToReturn->Data, ToReturn->MaxData * sizeof(char *));
            }
            ToReturn->Data[ToReturn->DataSize - 1] = new char[sizeof(fullPath)];
            strcpy(ToReturn->Data[ToReturn->DataSize - 1], fullPath);
        }
    }
    closedir(dir);
}

bool FilesSame(char *path1, char *path2){
    //Check if the files are the same as described in the assignment
    struct stat fileStat1;
    struct stat fileStat2;
    if(stat(path1, &fileStat1) < 0){
        cerr << "Error: " << path1 << " is not a valid path" << endl;
        return true;
    }
    if(stat(path2, &fileStat2) < 0){
        cerr << "Error: " << path2 << " is not a valid path" << endl;
        return true;
    }
    //Check if the files have the same size
    if (fileStat1.st_size != fileStat2.st_size){
        return false;
    }
    
    //Check if the files have the same content using open system call and read
    int fd1 = open(path1, O_RDONLY);
    int fd2 = open(path2, O_RDONLY);
    char buffer1[1];
    char buffer2[1];
    while(read(fd1, buffer1, 1) > 0 && read(fd2, buffer2, 1) > 0){
        //Bytewise comparison
        if(buffer1[0] != buffer2[0]){
            return false;
        }
    }
    return true;
}
bool LinksSame(char *path1, char *path2) {
    char buffer1[PATH_MAX];
    char buffer2[PATH_MAX];
    struct stat fileStat1, fileStat2;

    // Read and resolve the target of the first link
    ssize_t len1 = readlink(path1, buffer1, sizeof(buffer1) - 1);
    if (len1 == -1) {
        perror("Error reading link for path1");
        return false;
    }
    buffer1[len1] = '\0';

    // Read and resolve the target of the second link
    ssize_t len2 = readlink(path2, buffer2, sizeof(buffer2) - 1);
    if (len2 == -1) {
        perror("Error reading link for path2");
        return false;
    }
    buffer2[len2] = '\0';

    // Get file information of the resolved paths
    if (stat(buffer1, &fileStat1) < 0 || stat(buffer2, &fileStat2) < 0) {
        perror("Error statting resolved paths");
        return false;
    }

    // Compare inode 
    return (fileStat1.st_ino == fileStat2.st_ino);
}
void CheckDirectories(char *path1, char *path2, DataReturned * ToReturn){
    //This function will find the paths of the files that are unique in the two directories
    //The paths are stored in the struct DataReturned
    //The function is called recursively
    //The ToReturn struct is passed as an argument to the function
    //The function will add the paths of the files in the directory to the struct
    DIR *dir1 = opendir(path1);
    DIR *dir2 = opendir(path2);
    if (dir1 == NULL || dir2 == NULL){
        cerr << "Error: " << path1 << " or " << path2 << " is not a valid path" << endl;
        return ;
    }
    struct dirent *d1, *d2;
    bool flag;
    //The idea is to check first all the files that exist in the first directory and not in the second
    //Then check all the files that exist in the second directory and not in the first
    while ((d1 = readdir(dir1)) != NULL){
        //Read the files in the first directory
        if (strcmp(d1->d_name, ".") == 0 || strcmp(d1->d_name, "..") == 0) {
            // Skip the current and parent directory entries
            flag = false;
            continue;
        }
        //Use stat to open the file
        //Construct the path of the file
        char *path1new = new char[strlen(path1) + strlen(d1->d_name) + 2];
        strcpy(path1new, path1);        //Copy the path of the directory
        strcat(path1new, "/");      //Add a slash
        strcat(path1new, d1->d_name);    //Add the name of the file
        struct stat fileStat1;
        if(stat(path1new, &fileStat1) < 0){
            cerr << "Error1: " << path1new << " is not a valid path" << endl;
            return  ;
        }
        flag = true;        //If this remains true, then the file is unique
        rewinddir(dir2);    //Rewind the directory 2
        while (d2 = readdir(dir2)){
            //Read all the files in the second directory and compare them with the file in the first directory
            if (strcmp(d2->d_name, ".") == 0 || strcmp(d2->d_name, "..") == 0) {
            // Skip the current and parent directory entries
                continue;
            }
            //Same procedure as above to construct the path of the file d2
            char *path2new = new char[1024];
            strcpy(path2new, path2);
            strcat(path2new, "/");
            strcat(path2new, d2->d_name);
            
            struct stat fileStat2;
            if(stat(path2new, &fileStat2) < 0){
                cerr << "Error2: " << path2new << " is not a valid path" << endl;
                return  ;
            }
            //Check if the files are the same type
            if ((fileStat1.st_mode & S_IFMT) != (fileStat2.st_mode & S_IFMT)){
                //If they are not, then they are not the same
                continue;
            }
            //Check if they are files
            //Keep in mind that we check if the names of the files are the same here, not in the function FilesSame etc
            if (S_ISREG(fileStat1.st_mode)){
                if (strcmp(d1->d_name, d2->d_name) != 0){
                    continue;
                }
                
                if (FilesSame(path1new, path2new)){
                    flag = false;
                    break;
                }
            }
            //Check if they are directories
            else if (S_ISDIR(fileStat1.st_mode)){
                if (strcmp(d1->d_name, d2->d_name) != 0){
                    continue;
                }
                flag = false;
                //Recursively call the function
                CheckDirectories(path1new, path2new,ToReturn);
                break;
            }
            //Check if they are links
            else if (S_ISLNK(fileStat1.st_mode)){
                if (strcmp(d1->d_name, d2->d_name) != 0){
                    continue;
                }
                if (LinksSame(path1new, path2new)){
                    flag = false;
                    break;
                }
            }
            else{
                continue;
            }
        }
        if (flag){
            //If they are different, add the path of the file to the struct
            ToReturn->DataSize++;
            if (ToReturn->DataSize > ToReturn->MaxData){
                //If the size of Data is bigger than MaxData we increase the size of Data by a factor of 2
                ToReturn->MaxData = ToReturn->MaxData * 2;
                ToReturn->Data = (char **) realloc(ToReturn->Data, ToReturn->MaxData * sizeof(char *));
            }
            ToReturn->Data[ToReturn->DataSize - 1] = new char[sizeof(path1new)];
            strcpy(ToReturn->Data[ToReturn->DataSize - 1], path1new);
            //Copy path1new to the struct
            if (S_ISDIR(fileStat1.st_mode))     //If the file is a directory, call TraverseDirectory in order to add the paths of the files in the directory
                TraverseDirectory(path1new,ToReturn);
        }

    }
    rewinddir(dir2);
    rewinddir(dir1);
    //Same procedure as above but for the files in the second directory
    while ((d2 = readdir(dir2)) != NULL){
        if (strcmp(d2->d_name, ".") == 0 || strcmp(d2->d_name, "..") == 0) {
            // Skip the current and parent directory entries
            flag = false;
            continue;
        }
        //Use stat to open the file
        char *path2new = new char[1024];
        strcpy(path2new, path2);
        strcat(path2new, "/");
        strcat(path2new, d2->d_name);
        struct stat fileStat2;
        if(stat(path2new, &fileStat2) < 0){
            cerr << "Error3: " << path2new << " is not a valid path" << endl;
            return  ;
        }
        flag = true;
        rewinddir(dir1);
        while (d1 = readdir(dir1)){
            if (strcmp(d1->d_name, ".") == 0 || strcmp(d1->d_name, "..") == 0) {
            // Skip the current and parent directory entries
                continue;
            }
            char *path1new = new char[1024];
            strcpy(path1new, path1);
            strcat(path1new, "/");
            strcat(path1new, d1->d_name);
            struct stat fileStat1;
            if(stat(path1new, &fileStat1) < 0){
                cerr << "Error4: " << path1new << " is not a valid path" << endl;
                return ;
            }
            //Check if the files are the same type
            if ((fileStat1.st_mode & S_IFMT) != (fileStat2.st_mode & S_IFMT)){
                continue;
            }
            //Check if they are files
            if (S_ISREG(fileStat1.st_mode)){
                if (strcmp(d1->d_name, d2->d_name) != 0){
                    continue;
                }
                if (FilesSame(path1new, path2new)){
                    flag = false;
                    break;
                }
            }
            //Check if they are directories
            else if (S_ISDIR(fileStat1.st_mode)){
                
                if (strcmp(d1->d_name, d2->d_name) != 0){
                    continue;
                }
                flag = false;
                //CheckDirectories(path1new, path2new);
                break;
            }
            //Check if they are links
            else if (S_ISLNK(fileStat1.st_mode)){
                if (strcmp(d1->d_name, d2->d_name) != 0){
                    continue;
                }
                if (LinksSame(path1new, path2new)){
                    flag = false;
                    break;
                }
            }
            else{
                continue;
            }
        }
        if (flag){
            ToReturn->DataSize++;
            if (ToReturn->DataSize > ToReturn->MaxData){
                ToReturn->MaxData = ToReturn->MaxData * 2;
                ToReturn->Data = (char **) realloc(ToReturn->Data, ToReturn->MaxData * sizeof(char *));
            }
            ToReturn->Data[ToReturn->DataSize - 1] = new char[sizeof(path2new)];
            strcpy(ToReturn->Data[ToReturn->DataSize - 1], path2new);
            if (S_ISDIR(fileStat2.st_mode))
                TraverseDirectory(path2new,ToReturn);
        }
    }
    return;
}


void Compare(char *path1, char *path2) {
    //The compare function
    struct stat fileStat1;
    struct stat fileStat2;
    
    if(stat(path1, &fileStat1) < 0){
        cerr << "Error: " << path1 << " is not a valid path" << endl;
        return;
    }
    if(stat(path2, &fileStat2) < 0){
        cerr << "Error: " << path2 << " is not a valid path" << endl;
        return;
    }

    //Check if the files are the same type
    if ((fileStat1.st_mode & S_IFMT) != (fileStat2.st_mode & S_IFMT)){
        cout << "Expected two directories" << endl;
        return;
    }
    //Check if they are directories
    DataReturned * Returned = new DataReturned;
    Returned->DataSize = 0;
    Returned->MaxData = MAX;
    Returned->Data = (char **) malloc(MAX * sizeof(char *));
    if (S_ISDIR(fileStat1.st_mode))
        CheckDirectories(path1, path2, Returned);
    else{
        cout << "Expected two directories" << endl;
        return;
    }
    //properly call printStringsStartingWith in order to print the paths of the files that are different for each directory
    printStringsStartingWith(Returned->Data, Returned->DataSize, path1);
    printStringsStartingWith(Returned->Data, Returned->DataSize, path2);
}

void CopyFile(const char * source, const char * destination){
    //Simple bytewise copy of a file
    int fd1 = open(source, O_RDONLY);
    int fd2 = open(destination, O_WRONLY | O_CREAT, 0666);
    char buffer[1];
    while(read(fd1, buffer, 1) > 0){
        write(fd2, buffer, 1);
    }
    close(fd1);
    close(fd2);
}

void CopyLink(const char * source, const char * destination){
    //simple copy of a link
    char buffer[1024];
    int len = readlink(source, buffer, sizeof(buffer)-1);
    if (len == -1) {
        perror("readlink");
        return;
    }
    buffer[len] = '\0';
    if ( symlink(buffer, destination) == -1){
        perror("symlink");
        return;
    }
}

void CopyDirectory(const char * source, const char * destination){
    //This function is used to copy a directory that its path is source to a directory that its path is destination
    //The directory in destination is created in this funciton
    DIR* dir = opendir(source);
    if (dir == NULL) {
        cerr << "Error opening directory" << endl;
        return;
    }
    mkdir(destination, 0777);   //Create the directory in destination
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip the "." and ".." entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        // Construct full path for the source and destination
        char *src_path = new char[strlen(source) + strlen(entry->d_name) + 2]; // +1 for '/' and +1 for '\0'
        char *dest_path = new char[strlen(destination) + strlen(entry->d_name) + 2];
        //The source name is the name of the source directory + the name of the file
        snprintf(src_path, strlen(source) + strlen(entry->d_name) + 2, "%s/%s", source, entry->d_name);
        //The destination name is the name of the destination directory + the name of the file
        snprintf(dest_path, strlen(destination) + strlen(entry->d_name) + 2, "%s/%s", destination, entry->d_name);
        struct stat statbuf;
        stat(src_path, &statbuf);
        // Check what type of file it is and call the appropriate function
        if (S_ISDIR(statbuf.st_mode)) {
            mkdir(dest_path, statbuf.st_mode);
            CopyDirectory(src_path, dest_path);
        } 
        else if (S_ISREG(statbuf.st_mode)){
            CopyFile(src_path, dest_path);
        }
        else if (S_ISLNK(statbuf.st_mode)){
            CopyLink(src_path, dest_path);
        }
    }
}


void copyFiles(char *paths[], const char *path1, const char *destFolder, int numFiles,const char * path2) {
    //This function is used to copy the files that are unique in the two directories
    //The paths of the files are stored in the array paths
    //The path of the first directory is path1
    //The path of the second directory is path2
    //The path of the destination directory is destFolder
    //The number of files is numFiles
    //This function is useful when we want to merge two directories
    //The idea is that first we copy the file of the first directory to the destination directory
    //Then we copy the unique files of the second directory to the destination directory
    //However, if a file exists in both directories, we check which one is the most recently modified
    //This funciton deals with this problem as it will be described below
    size_t path1_len = strlen(path1);
    for (int i = 0; i < numFiles; ++i) {        //For every file in the array paths
        if (strncmp(paths[i], path1, path1_len) == 0 && paths[i][path1_len] == '/')
            //If the file is in the first directory, then it has already been copied, so we continue
            continue;
        //If we are here, then the file is in the second directory
        //Construct the path of the file in the destination directory
        //The path will be the path of the destination directory + the name of the file 
        char * destPath = new char[strlen(destFolder) + strlen(paths[i]) - strlen(path2) + 1];
        snprintf(destPath, strlen(destFolder) + strlen(paths[i]) - strlen(path2) + 1, "%s%s", destFolder, paths[i] + strlen(path2));
        //Chek if a file exists in destPath
        //If it does, delete it

        struct stat fileStat;
        struct stat path_stat;
        lstat(paths[i], &path_stat);    //Get the information of the file that is going to be copied
        if(lstat(destPath, &fileStat) >= 0){    //Check if a file exists in destPath
            //if it does, check if it is more recently modified than the file that is going to be copied
            //To do tis we need to construct the path of the file in the first directory and get its information
            //The path will be the path of the first directory + the name of the file
            char * PathToFolder1 = new char[strlen(paths[i]) - strlen(path2) + strlen(path1) + 1];
            snprintf(PathToFolder1, strlen(paths[i]) - strlen(path2) + strlen(path1) + 1, "%s%s", path1, paths[i] + strlen(path2));
            struct stat fileStat1;  //The information of the file in the first directory
            lstat(PathToFolder1, &fileStat1);
            if (fileStat1.st_mtime > path_stat.st_mtime){
                //If the file in the first directory is more recently modified, then the file in destPath is kept and nothing is done
                continue;
            }
            //If we are here, then the file in the second directory is more recently modified and need to be copied
            //However, since a file already exists in destPath, we need to delete the file in destPath first
            //To do this we need to check the type of the file in destPath and delete it accordingly
            if (S_ISDIR(fileStat1.st_mode)){
                rmdir(destPath);
            }
            else if (S_ISREG(fileStat1.st_mode)){
                unlink(destPath);
            }
            else if (S_ISLNK(fileStat1.st_mode)){
                unlink(destPath);
            }
        }
        //In any case, if we are here, then no file exists in destPath and we can copy the file from the second directory to destPath
        //Check the type of the file and call the appropriate function
        if (S_ISDIR(path_stat.st_mode)) {
            mkdir(destPath, path_stat.st_mode);
        } else if (S_ISLNK(path_stat.st_mode)) {
            CopyLink(paths[i], destPath);
        } else {
            CopyFile(paths[i], destPath);
        }

    }
}



void MergeFiles(char * path1,char * path2,char * Destination){
    struct stat fileStat1;
    struct stat fileStat2;
    
    if(stat(path1, &fileStat1) < 0){
        cerr << "Error: " << path1 << " is not a valid path" << endl;
        return;
    }
    if(stat(path2, &fileStat2) < 0){
        cerr << "Error: " << path2 << " is not a valid path" << endl;
        return;
    }

    //Check if the files are the same type
    if ((fileStat1.st_mode & S_IFMT) != (fileStat2.st_mode & S_IFMT)){
        cout << "Expected two directories" << endl;
        return;
    }
    DataReturned * Returned = new DataReturned;
    Returned->DataSize = 0;
    Returned->MaxData = MAX;
    Returned->Data = (char **) malloc(MAX * sizeof(char *));
    if (S_ISDIR(fileStat1.st_mode))
        CheckDirectories(path1, path2, Returned);
        //Use the function CheckDirectories to find the paths of the files that are unique in the two directories
    else{
        cout << "Expected two directories" << endl;
        return;
    }
    CopyDirectory(path1, Destination);    //Copy the first directory to the destination directory
    copyFiles(Returned->Data, path1, Destination, Returned->DataSize,path2);    //Copy the files that are unique in the two directories
    //So the final folder is the first directory + the files that are unique in the second directory
}