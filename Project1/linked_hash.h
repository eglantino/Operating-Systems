#ifndef LINKED_HASH

    #define LINKED_HASH
#include "voters.h"
#include "reverse_list.h"

    typedef Voter Item;
    int InitializeHash(int KeysPerBucket,int );
    void Insert(Item * item);
    int Find(int Pin);
    int ChangeItem(int Pin);
    int ExitHash(void);
    Item * FindRecord(int);
#endif 
