#include "linked_hash.h"
#include "voters.h"
#include <iostream>
#include <math.h>


#define ERROR -1
using namespace std;

static int KeysPerBucket =0;
class HashTable;
static HashTable * MyHash = NULL;
int BytesDeleted = 0;
class Bucket {
    public :
        Item ** Records;                    
        //An array of indexes to Items, when this array reaches KeysPerBucket indexes, a new bucket is created and linked with the current bucket via NextBucker
        Bucket * NextBucket;        //Pointer to next bucket
        int ItemsStored;            //#items stored in bucket
        
        Bucket() : ItemsStored(0){
            if ( (Records = new Item*[KeysPerBucket]) == NULL){
                cout << "Could not allocate memory\n";
                return;
            }
            for (int i = 0 ; i < KeysPerBucket ; i++ )
                Records[i] = NULL;
            NextBucket = NULL;
        }
        void InsertIem(Item * item){
            if (ItemsStored == KeysPerBucket){            //Bucket is full
                if (NextBucket == NULL)                   //if there is no next bucket, create one
                    if ( (NextBucket = new Bucket)  == NULL){
                        cout << "Could not allocate memory\n";
                        return;
                    }
                NextBucket->InsertIem(item);                //Insert the item to the bucket created
            }
            else{
                Records[ItemsStored++] = item;              //Bucket is not full, insert to the free space
            }
        }
        Bucket * Split();            //Will be defined later
        int Find(int Pin){
            for(int i = 0 ; i< KeysPerBucket;i++){       //Check all records in the bucket
                if(Records[i] == NULL)
                    return ERROR;
                if (Records[i]->GetPin() == Pin){
                    Records[i]->DisplayVoter();
                    return 0;
                }
            }
            if (NextBucket == NULL)
                return ERROR;
            return NextBucket->Find(Pin);    // If not foun yet check next bucket
        }
        Item * FindRecord( int Pin){         // Function that given pin returns item
            //This is needed when vote is changed to yes
            //given pin the appropriate item needs to be inserted to the inverted list
            for(int i = 0 ; i< KeysPerBucket;i++){
                if(Records[i] == NULL)          
                //We check all the records of the bucket, and since last record was NUULL there is no Next Bucket
                    return NULL;
                if (Records[i]->GetPin() == Pin){
                    return Records[i];
                }
            }
            if (NextBucket == NULL)
                return NULL;
            return NextBucket->FindRecord(Pin);
        }
        int Change(int Pin){            
            //Function that changes some data for the canditate with the given Pin
            //What is changed is actually thw vote from no to yes, this can easy change by modyfing SetVote to smth else
            for(int i = 0 ; i< KeysPerBucket;i++){
                if(Records[i] == NULL)
                    return ERROR;
                if (Records[i]->GetPin() == Pin)
                    return Records[i]->SetVote();
            }
            if (NextBucket == NULL)
                return ERROR;
            return NextBucket->Change(Pin);
        }
        ~Bucket(){      //Destructor that frees the memory for the bucket and also destroys all Voter items
            for (int i = 0 ; i< KeysPerBucket;i++){
                if (Records[i] == NULL)
                    break;
                BytesDeleted += int(sizeof(Records[i]));
                delete Records[i];
            }
            BytesDeleted += int(sizeof(Records));
            delete [] Records;
            if (NextBucket != NULL){
                BytesDeleted += int(sizeof(NextBucket));
                delete NextBucket;
            }
        }
};

class HashTable {
    // Hash table class
    private: 
        int round;
        int PrevSize;
        Bucket ** HashBackets;      
        //Dynamic array of pointers to Buckets, each cell of the hash table defines a pointer to a bucket
        //Each bucket may be connected, as linked list, with other buckets (overflow buckets) that belong to the same cell
    public : 
        int NextSplit;      //Index of the cell that may be splitted next
        int Size;
        int HashValue1;     //H1
        int HashValue2;     //H2
        int TotalRecords;
        HashTable(int InitialSize) : round(0), PrevSize(InitialSize),NextSplit(0), Size(InitialSize),  TotalRecords(0) {
            if ( ( HashBackets = (Bucket **) malloc(sizeof(Bucket *) * Size)) == NULL){
                cout << "Could not allocate memory\n";
                return;
            }
            for(int i = 0 ; i < Size ; i++ )
                if ( (HashBackets[i] = new Bucket) == NULL ){
                    cout << "Could not allocate memory\n";
                    return;
                }
            HashValue1 = (int)(pow(2,round)) * Size;            //Initial values of h1 and h2, same as the paper
            HashValue2 = (int)(pow(2,round + 1)) * Size;
        }
        void InsertItem(int BucketPos,Item * item){
            //Insert to the indexed bucket
            HashBackets[BucketPos]->InsertIem(item);
        
        }
        void Split(){
            if ( ( HashBackets = (Bucket**) realloc(HashBackets, sizeof(Bucket *)* (++Size)) ) == NULL){
                //Allocate memory and create a new bucket
                cout << "Could not allocate memory\n";
                return;
            }
            if ( (HashBackets[Size-1] = new Bucket) == NULL){
                cout << "Could not allocate memory\n";
                return;
            }
            HashBackets[NextSplit] = HashBackets[NextSplit]->Split();
            //Split the apropirate bucket
            //Since Split is made, we need to de-allocate memory, if a bucket has empted
            //Also we need to check whether we are going to next round
            Bucket * NextBucket = HashBackets[NextSplit]->NextBucket, * PrevBucket = HashBackets[NextSplit];
            if (NextBucket != NULL){
                do
                {
                    //Find all buckets that have no itmes and delete them
                    if(NextBucket->ItemsStored == 0){
                        PrevBucket->NextBucket = NULL;
                        delete NextBucket;
                        break;
                    }
                    else{
                        PrevBucket = NextBucket;
                        NextBucket = NextBucket->NextBucket;
                    }
                } while (NextBucket != NULL);
            }
            if (Size == 2*PrevSize){
                //New round
                HashValue1 *=2 ;
                HashValue2 *= 2;
                round++;
                NextSplit = 0;
                PrevSize = Size;
                
            }
            else
                NextSplit++;
            
            
        }
        int Find(int ItemPos, int Pin){
            //Tries to find item with Pin in bucket indexed by item Pos, returns ERROR if no such Pin
            return HashBackets[ItemPos]->Find(Pin);
        }
        Item * FindRecord(int ItemPos, int Pin){
            //Same functionality as above but now returns a pointer to that item
            return HashBackets[ItemPos]->FindRecord(Pin);
        }
        int Change(int ItemPos, int Pin){
            //Perform change
            return HashBackets[ItemPos]->Change(Pin);
        }
        ~HashTable(){
            for(int i = 0 ; i< Size; i++){
                BytesDeleted += int(sizeof(HashBackets[i]));
                delete HashBackets[i];
            }
            BytesDeleted += int(sizeof(HashBackets));
            free(HashBackets);
        }
};

Bucket * Bucket::Split(){
    // The bucket Split, we delete all records and we insert them again to the updated hash table
    Item * temp;
    ItemsStored = 0;
    for (int i = 0; i< KeysPerBucket;i++){
        temp = Records[i];
        if (temp == NULL)
            break;
        int ItemPos = temp->GetPin() % MyHash->HashValue2;
        Records[i] =NULL;
        MyHash->InsertItem(ItemPos,temp);
    }
    if(NextBucket != NULL)
        NextBucket = NextBucket->Split();
    return this;
}


int InitializeHash (int x,int InitialSize){
    //Simple initialization
    if (x <= 0){
        cout << "Keys per Bucket must be an integer greater than zero, you inserted " << x << "\n";
        return ERROR;
    }
    if (InitialSize <= 0 ){
        cout << "Initial Size must be an integer greater than zero, you inserted " << x << "\n";
        return ERROR;
    }
    KeysPerBucket = x;
    if ( ( MyHash = new HashTable(InitialSize)) == NULL){
        cout << "Could not allocate memory\n";
        return ERROR;
    }
    return 0;
}

void Insert (Item  * item) {
    int ItemPos = item->GetPin() % MyHash->HashValue1;
    if (ItemPos < MyHash->NextSplit)
    //Check id index of the insertion is smaller that index of the next split
    //If so use h2
        ItemPos = item->GetPin() % MyHash->HashValue2;
    MyHash->InsertItem(ItemPos,item);
    MyHash->TotalRecords++;
    double L = (double)MyHash->TotalRecords/ (double)(MyHash->Size * KeysPerBucket);
    if (L > 0.75)
        MyHash->Split();    
}

int Find(int Pin){          // Returns 0 if record is displayed ERROR otherwise
    int ItemPos = Pin % MyHash->HashValue1;
    if (ItemPos < MyHash->NextSplit)
        ItemPos = Pin % MyHash->HashValue2;
    int RetrurnValue = MyHash->Find(ItemPos,Pin);
    if (RetrurnValue == ERROR){
        cout << "Participant " << Pin << " not in cohort\n";
        return ERROR;
    }
    return 0;
}
Item * FindRecord(int Pin){
    int ItemPos = Pin % MyHash->HashValue1;
    if (ItemPos < MyHash->NextSplit)
        ItemPos = Pin % MyHash->HashValue2;
    return MyHash->FindRecord(ItemPos,Pin);
}
int ChangeItem(int Pin){
    int ItemPos = Pin % MyHash->HashValue1;
    if (ItemPos < MyHash->NextSplit)
        ItemPos = Pin % MyHash->HashValue2;
    if (MyHash->Change(ItemPos,Pin) == ERROR)
        return ERROR;
    Item * ToInsert = FindRecord(Pin);
    if (ToInsert == NULL){
        cout << "There is no candiatte with Pin : " << Pin << "\n";
        return ERROR;
    }
    InsertList(ToInsert);
    return 0;
}

int ExitHash(void){
    BytesDeleted = (int)(sizeof(MyHash));
    delete MyHash;
    return BytesDeleted;
}
