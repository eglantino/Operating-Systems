#include "utils.h"
#include <string.h>

int RecordStrCmp(MyRecord A, MyRecord B){
    int x = strcmp(A.LastName,B.LastName);
    if ( x == 0){
        x = strcmp(A.FirstName,B.FirstName);
        if (x == 0 ){
            if ( A.custid < B.custid)
                return -1;
            return 1;
        }
        else
            return x;
    }
    else
        return x;
    
}