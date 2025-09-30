#include <iostream>
#include "voters.h"
//Voters.h will be used to interact with the database
#include <string.h>
#include <error.h>

#define ERROR -1
#define STRING_LENGTH 100
#define INITIAL_SIZE 2

using namespace std;

void ShowPrompt( void ){
    cout << "Please enter any of the following orders, as many times as you want to perform the corresponding task : \n ";
    cout << "1 <pin>    (Find voter with pin as id and display)\n";
    cout << "i <pin> <lname> <fname> <zip>   (Insert New Voter)\n";
    cout << "m <pin>    (Mark as voted)\n";
    cout << "bv <fileofkeys>    (Set voters of file as voted)\n";
    cout << "v    (Number of people that voted)\n";
    cout << " perc   (Percentage of people that voted)\n";
    cout << "z <zipcode>   (Voters of a specific zipcode)\n";
    cout << "o   (Display all zipcodes )\n";
    cout << "prompt (Show prompt)\n";
    cout << "exit\n";

}


void Operate(void) {
    char * Input,**tokens;
    if ( (Input = (char*)malloc(sizeof(char)*STRING_LENGTH)) == NULL){         //Holds the user's input
        cout << "Could not allocate memory\n";
        return;
    }           
    if ( ( tokens =(char **) malloc(6*sizeof(char *))) == NULL){          //Break the input to maximum 6 tokens, more than 6 will not correspond to a valid operation
        cout << "Could not allocate memory\n";
        return;
    }
    while (1)
    {
        fgets(Input, STRING_LENGTH,stdin);
        int i = 0;
        if (Input[strlen(Input)-1] == '\n')
            Input[strlen(Input)-1] ='\0';
        tokens[i] = strtok(Input," ");
        while (tokens[i] != NULL)
        {
            i++;
            if (i == 6){
                cout << "Too Many Arguments\n";
                break;
            }
            tokens[i] = strtok(NULL," ");
        }
        if ( i == 6)
            continue;
        if (strcmp(tokens[0],"l") == 0){
            if ( i != 2){
                cout << "Wrong number of arguments in input\n";
                continue;
            }
            bool flag = false;
            for (int i = 0; i < (int)strlen(tokens[1]); i++) {         
                 //A simple way to check if token is positive number, checking each char if is digit or not
                if(!isdigit(tokens[1][i])){
                    cout<< "Malformed Pin\n";
                    flag = true;
                    break;
                }
            }
            if(flag)
                continue;
            PrintVoter(atoi(tokens[1]));
        }
        else if (strcmp(tokens[0],"i") == 0)
        {
            if ( i != 5){
                cout << "Wrong number of arguments in input\n";
                continue;
            }
            bool flag = false;
            for (int i = 0; i < (int)strlen(tokens[1]); i++) {      //Token[1] is pin
                if(!isdigit(tokens[1][i])){
                    cout<< "Malformed Input\n";
                    flag = true;
                    break;
                }
            }
            if(flag)
                continue;
            flag = false;
            for (int i = 0; i < (int)strlen(tokens[4]); i++) { // Token[4] is zip code
                if(!isdigit(tokens[4][i])){
                    cout<< "Malformed Input\n";
                    flag = true;
                    break;
                }
            }
            if(flag)
                continue;
            
            for (int i = 0; i < (int)strlen(tokens[2]); i++) {      //Token[2] is lname, no digits approved
                if(isdigit(tokens[2][i])){
                    cout<< "Malformed Input\n";
                    flag = true;
                    break;
                }
            }
            if(flag)
                continue;
            flag = false;
            for (int i = 0; i < (int)strlen(tokens[3]); i++) { // Token[3] is fname, no digits approved
                if(isdigit(tokens[3][i])){
                    cout<< "Malformed Input\n";
                    flag = true;
                    break;
                }
            }
            if(flag)
                continue;    
            


            if ( CreateVoter(atoi(tokens[1]),tokens[2],tokens[3],atoi(tokens[4])) == ERROR)     
            //Create Voter returns ERROR if voter exists, 0 otherwise
                cout << tokens[1] << " already exist\n";
            else
                cout << "Inserted" << " " << tokens[1] << " " << tokens[2] << " " << tokens[3]<< " "  << tokens[4] << " N\n" ;
        }
        else if (strcmp(tokens[0],"m") == 0)
        {
            if ( i != 2){
                cout << "Wrong number of arguments in input\n";
                continue;
            }
            bool flag = false;
            for (int i = 0; i < (int)strlen(tokens[1]); i++) {
                if(!isdigit(tokens[1][i])){
                    cout<< "Malformed Input\n";
                    flag = true;
                    break;
                }
            }
            if(flag)
                continue;
            if ( SetVoted(atoi(tokens[1])) == ERROR)
            ///SetVoted returns ERROR if voter exists, 0 otherwise
                cout << tokens[1] << " does not exist\n";
        }
        else if (strcmp(tokens[0],"bv") == 0)
        {
            if ( i != 2){
                cout << "Wrong number of arguments in input\n";
                continue;
            }
            FILE* ptr;
            ptr = fopen(tokens[1], "r");
            if (ptr != NULL){
                char * s1;
                if  (( s1= (char*)malloc(STRING_LENGTH*sizeof(char)) ) == NULL){
                    cout << "Could not allocate memory\n";
                    return;
                }
                while(fgets(s1, STRING_LENGTH, ptr)){
                    if (s1[strlen(s1)-1] == '\n')
                        s1[strlen(s1)-1]='\0';
                    bool flag = false;
                    for (int i = 0; i < (int)strlen(s1); i++) {
                        if(!isdigit(s1[i])){
                            cout<< "Malformed Input\n";
                            flag = true;
                            break;
                        }
                    }
                    if(flag)
                        break;;
                    if ( SetVoted(atoi(s1)) == ERROR)
                        cout << s1 << " does not exist\n";
                }
                free(s1);
                fclose(ptr);
            }
            else
                cout << tokens[1]<< " could not be opened\n";
            
        }
        else if (strcmp(tokens[0],"v") == 0)
        {
            if ( i != 1){
                cout << "Wrong number of arguments in input\n";
                continue;
            }
            cout << "Voted so far " << NumberOfYesVoters() << "\n";
        }
        else if (strcmp(tokens[0],"o") == 0)
        {
            if ( i != 1){
                cout << "Wrong number of arguments in input\n";
                continue;
            }
            PrintPostalCodes(); 
        }
        else if (strcmp(tokens[0],"exit") == 0)
        {
            if ( i != 1){
                cout << "Wrong number of arguments in input\n";
                continue;
            }
            free(tokens);
            cout << "Exitng...\n";
            cout << ExitProg() << " of Bytes Released\n";
            break;
        }
        else if (strcmp(tokens[0],"perc") == 0)
        {
            if ( i != 1){
                cout << "Wrong number of arguments in input\n";
                continue;
            }
            cout << ((double)NumberOfYesVoters()/(double)NumberOfVoters())*100 << " %\n";

        }
        else if (strcmp(tokens[0],"z") == 0)
        {
            if ( i != 2){
                cout << "Wrong number of arguments in input\n";
                continue;
            }
            bool flag = false;
            for (int i = 0; i < (int)strlen(tokens[1]); i++) {      //Token[1] is zip
                if(!isdigit(tokens[1][i])){
                    cout<< "Malformed Input\n";
                    flag = true;
                    break;
                }
            }
            if(flag)
                continue;
            PrintAllFromZip(atoi(tokens[1]));
        }
        else if (strcmp(tokens[0],"prompt") == 0){          //Prompt addition
            if ( i != 1){
                cout << "Wrong number of arguments in input\n";
                continue;
            }
            ShowPrompt();
        }
        else {
            cout << "Wrong input!\n";
        }
        
    }
    free(Input);
}
int main(int argc, char **argv){
    if (argc != 5 && argc != 7){
        //Program only accepts 2 or 3 flags with a value following them
        cout << "Wrong number of arguments!\n";
        return ERROR;
    }
    if (argc == 7){
        //we have 3 flags, we check whether any 2 of them are same
        if ((strcmp(argv[1],argv[3]) == 0) || (strcmp(argv[3],argv[5]) == 0) || (strcmp(argv[1],argv[5]) == 0)){
            cout << "Wrong arguments\n";
            return ERROR;
        }
    }
    else{
        //we have 2 flags, check if they are same
        if (strcmp(argv[1],argv[3]) == 0){
            cout << "Wrong arguments\n";
            return ERROR;
        }
    }
    char * InputFile = NULL,*KeysPerBucket = NULL, *InitialSize = NULL;
    for(int i = 1; i<argc; i+=2){
        //Check all flags and get their value
        if (strcmp(argv[i],"-f") == 0){
            if ( ( InputFile = (char*)malloc(sizeof(char)*strlen(argv[i+1])+1) ) ==NULL){
                cout << "Could not allocate memory\n";
                return ERROR;
            }
            strcpy(InputFile,argv[i+1]);
        }
        else if(strcmp(argv[i],"-b") == 0){
            if ( ( KeysPerBucket = (char*)malloc(sizeof(char)*strlen(argv[i+1])+1) ) == NULL ){
                cout << "Could not allocate memory\n";
                return ERROR;
            }
            strcpy(KeysPerBucket,argv[i+1]);
        }
        else if(strcmp(argv[i],"-m") == 0){
            if ( ( InitialSize = (char*)malloc(sizeof(char)*strlen(argv[i+1])+1) ) == NULL ){
                cout << "Could not allocate memory\n";
                return ERROR;
            }
            strcpy(InitialSize,argv[i+1]);
        }
        else{
            cout << "Wrong arguments\n";
            return ERROR;
        }

    }
    if ( KeysPerBucket == NULL || InputFile == NULL) {
        //Flags that were needed didnt get initialized in the previous for loop
        cout << "-f and -b flags not found\n";
        return ERROR;
    }
    for (int i = 0; i < (int)strlen(KeysPerBucket); i++) {
      if(! isdigit(KeysPerBucket[i])){
        cout<<"Expected an int greater than zero for keys per bucket but got : " << KeysPerBucket << "\n";
        return ERROR;
      }
   }
   if (InitialSize != NULL){
        for (int i = 0; i < (int)strlen(InitialSize); i++) {
            if(! isdigit(InitialSize[i])){
                cout<<"Expected an int greater than zero for Initial Size but got : " << InitialSize << "\n";
                return ERROR;
            }
        }
        if (Initialize(atoi(KeysPerBucket),atoi(InitialSize)) == ERROR)
            return ERROR;
    }
    else{
        if (Initialize(atoi(KeysPerBucket), INITIAL_SIZE) == ERROR)
            return ERROR;
    }
    FILE* fp;
    fp = fopen(InputFile,"r");
    free(InitialSize);
    free(KeysPerBucket);
    char * s1,**tokens ;
    if ((s1= (char*)malloc(STRING_LENGTH*sizeof(char))) == NULL ){
        cout << "Could not allocate memory\n";
        return ERROR;
    }
    if (fp != NULL){
        int zip,pin;
        char * name, * surname;
        if ( ( name = (char*) malloc (sizeof(char)*STRING_LENGTH) ) == NULL){
            cout << "Could not allocate memory\n";
            return ERROR;
        }
        if ( (surname =(char*) malloc(sizeof(char)*STRING_LENGTH)) == NULL ){
            cout << "Could not allocate memory\n";
            return ERROR;
        }
        int i;
        if ( ( tokens =(char **) malloc(6*sizeof(char *))) == NULL){          //Break the input to maximum 6 tokens, more than 6 will not correspond to a valid operation
            cout << "Could not allocate memory\n";
            return ERROR;
        }
        while( fgets(s1, STRING_LENGTH, fp) ){
            i = 0;
            tokens[i] = strtok(s1," ");
            while (tokens[i] != NULL)
            {
                i++;
                if (i == 5){
                    cout << "Too Many Arguments while reading from "<< InputFile << "\n";
                    return ERROR;
                }
                tokens[i] = strtok(NULL," ");
            }
            
            pin = atoi(tokens[0]);
            strcpy(name,tokens[1]);
            strcpy(surname,tokens[2]);
            zip = atoi(tokens[3]);
            CreateVoter(pin,name,surname,zip);
        }
        free(name);
        free(surname);
        fclose(fp);
    }
    else{
        printf("Error %d \n", errno);
        cout << "Could not open file " << InputFile << "\n";
        return ERROR;
    }
    free(tokens);
    free(InputFile);
    free(s1);
    ShowPrompt();
    Operate();
    return 0;
}
