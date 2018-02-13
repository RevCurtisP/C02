/************************************
 * C02 Statement Compiling Routines *
 ************************************/

char asnvar[VARLEN+1];      //Assigned Variable Name
char asnidx[VARLEN+1] ;     //Assigned Variable Index
int  asnivt;                //Assigned Index Variable Type

char xstmnt[LINELEN];       //Required Statement

void bgnblk(char blkchr);    //End Program Block
void endblk(int blkflg);    //End Program Block
void pdowhl();              //Parse and Compile WHILE after DO
void prcasn(char trmntr);   //Process Assignment
void prcvar(char trmntr);   //Process Variable at Beginning of Statement
void prsasn(char trmntr);   //Parse and Compile and Assignment
void prssif(char trmntr);   //Parse Shortcut If
void pstmnt();              //Parse and Compile Program Statement
