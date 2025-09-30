#ifndef VOTERS

    #define VOTERS
    int Initialize (int KeysPerBucket,int InitialSize);
    int CreateVoter(int Pin, char * surname, char * name, int PostCode);
    int PrintVoter(int Pin);
    int SetVoted(int Pin);
    int NumberOfVoters (void);
    int NumberOfYesVoters (void);
    void PrintPostalCodes(void);
    int PrintAllFromZip(int Zip);
    int ExitProg(void);

    class Voter{
        public : 
            Voter(int Pin, const char * surname, const char * name, int PostCode);
            ~Voter();
            int GetPin();
            void DisplayVoter();
            int SetVote();
            int GetZip();
        private : 
            int Pin,PostCode;
            char *name,*surname;
            bool HasVoted;
    };


#endif 
