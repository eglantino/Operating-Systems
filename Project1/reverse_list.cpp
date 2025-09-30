#include "reverse_list.h"
#include <iostream>
#define ERROR -1
using namespace std;

struct ItemNode{
    Item * item;
    ItemNode * Next;
};

struct ZipNode{
    int Zip, NumberOfItems;
    ZipNode * Next;
    ItemNode * Items;
};
static int TotalZips = 0;       //Holds the total number of ZipNodes
static ZipNode * Head = NULL;   //Head of the ZipNode list

ZipNode * CreateNewZip(Item * item, ZipNode * Next,int Zip){
    //Function that allocates memory for a new ZipNode, initializes some values and return a pointer to it
    //It also gets as parameter a pointer to the item that is being inserted
    TotalZips++;
    ZipNode * ToReturn;
    if ((ToReturn = new ZipNode) ==NULL){
        cout << "Could not allocate memory\n";
        return NULL;
    }
    if ( (ToReturn->Items = new ItemNode) == NULL){
        cout << "Could not allocate memory\n";
        return NULL;
    }
    ToReturn->Items->item = item;
    ToReturn->Items->Next = NULL;
    ToReturn->Next = Next;
    ToReturn->NumberOfItems = 1;
    ToReturn->Zip = Zip;
    return ToReturn;
}

void InsertList(Item * item){
    if (Head == NULL){
        //No items have been inserted yet, we need to initialize the head
        TotalZips = 0;
        if ( (Head = CreateNewZip(item,NULL,item->GetZip())) == NULL){
            cout << "Could not allocate memory\n";
            return ;
        }
        return;
    }
    ZipNode * temp = Head,*PrevZip = NULL;
    while(temp != NULL){
        if(temp->Zip == item->GetZip()){
            //item needs to be inserted to temp ZipNode
            temp->NumberOfItems++;      //Insert it
            ItemNode * prev = temp->Items;
            ItemNode * next = prev->Next;
            while (next != NULL)
            {
                prev = next;
                next = next->Next;
                //Iterate through th lsit of Items that belong to temp zipnode and add it to the end of the list
            }
            if ((prev->Next = new ItemNode) ==NULL){
                cout << "Could not allocate memory\n";
                return ;
            }
            prev->Next->item = item;        //New Item Node initalization
            prev->Next->Next = NULL;
            if (PrevZip != NULL){          
                //If null we are in head node and we are done
                //since the zip node we inserted the item was already the one with the biggest number of items
                PrevZip->Next = temp->Next;
                ZipNode * Position2 = Head, * Position1 = NULL; 
                //After the next while loop, temp ZipNode need to be between Position1 and Position 2 ZipNodes
                while (Position2 != NULL && Position2->NumberOfItems >= temp->NumberOfItems){
                    //iterate ZipNode list untill node at positon 2 has smaller number of Items than the temp ZipNode
                    Position1 = Position2;
                    Position2 = Position2->Next;
                }
                if (Position1 == NULL) {            //Temp needs to be head node
                    Head = temp;
                    temp->Next = Position2;
                }
                else{
                    //Insert temp between the two nodes
                    Position1->Next = temp;
                    temp->Next = Position2;
                }

            }
            return;
        }
        PrevZip = temp;
        temp = temp->Next;

    }
    if ( (PrevZip->Next = CreateNewZip(item,NULL,item->GetZip())) == NULL){
        //Means that we reached end of list, and no ZipNode with item->zip exists
        cout << "Could not allocate memory\n";
        return ;
    }
}

int VotersFromZip(int Zip){
    //Find ZipNode with such zip
    ZipNode * temp = Head;
    while (temp != NULL)
    {
        if(temp->Zip == Zip){
            ItemNode * temp2 = temp->Items;
            std::cout << temp->NumberOfItems << " voted in " << Zip << "\n";
            while(temp2 != NULL){
                cout << temp2->item->GetPin() << "\n";
                temp2 = temp2->Next;
            }
            return 0;
        }
        temp = temp->Next;
    }
    std::cout << "There are no voters for the zip code " << Zip << ".\n";
    return ERROR;
}

void DisplayZips(void){
    //Simply print the ZipNode list
    if (Head == NULL){
        std::cout << "No one has voted yet!\n";
        return;
    }
    ZipNode * temp = Head ;
    while (temp!=NULL)
    {
        std::cout << temp->Zip << " : " << temp->NumberOfItems << "\n";
        temp = temp->Next;
    }
}



int ExitList(void){
    //Easy destruction of list
    int BytesDeleted = 0;
    ZipNode * temp = Head,*ToDelete;
    while (temp != NULL)
    {
        ItemNode * temp2 = temp->Items,*ToDelete2;
        while (temp2 != NULL)
        {
            ToDelete2 = temp2;
            temp2 = temp2->Next;
            BytesDeleted += (int)(sizeof(ToDelete2));
            delete ToDelete2;
        }
        ToDelete = temp;
        temp = temp->Next;
        BytesDeleted += (int)(sizeof(ToDelete));
        delete ToDelete;
    }
    return BytesDeleted;
}
