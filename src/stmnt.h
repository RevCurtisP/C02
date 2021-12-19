/************************************
 * C02 Statement Compiling Routines *
 ************************************/

char asnvar[VARLEN+1];      //Assigned Variable Name
int  asntyp;                //Assigned Variable Type
char asnidx[VARLEN+1] ;     //Assigned Variable Index
int  asnivt;                //Assigned Index Variable Type
char ysnvar[VARLEN+1];      //Assigned Y Variable Name
char ysnidx[VARLEN+1] ;     //Assigned Y Variable Index
int  ysnivt;                //Assigned Y Index Variable Type
char xsnvar[VARLEN+1];      //Assigned X Variable Name
char xsnidx[VARLEN+1] ;     //Assigned X Variable Index
int  xsnivt;                //Assigned X Index Variable Type

char xstmnt[LINELEN];       //Expected Statement

void bgnblk(char blkchr);    //End Program Block
void endblk(int blkflg);    //End Program Block
void prcasp(char trmntr);	//Process Pointer Assignment
void prcidx(int idxtyp, char *name, char *index);
void pdowhl();              //Parse and Compile WHILE after DO
void prssym();              //Parse and Compile Assignment or Function Call
void pstmnt();              //Parse and Compile Program Statement
