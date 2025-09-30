
#ifndef UTILS

    #define UTILS

    #define SIZEofBUFF 20
    #define SSizeofBUFF 6
    #define ERROR -1
    typedef struct{
        int  	custid;
        char 	LastName[SIZEofBUFF];
        char 	FirstName[SIZEofBUFF];
        char	postcode[SSizeofBUFF];
    } MyRecord;
    int RecordStrCmp(MyRecord A, MyRecord B);
#endif