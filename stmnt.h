/************************************
 * C02 Statement Compiling Routines *
 ************************************/

char asnvar[VARLEN+1];      //Assigned Variable Name
char asnidx[VARLEN+1] ;     //Assigned Variable Index
int  asnivt;                //Assigned Index Variable Type

char xstmnt[LINELEN];       //Expected Statement

void bgnblk(char blkchr);    //End Program Block
void endblk(int blkflg);    //End Program Block
void pdowhl();              //Parse and Compile WHILE after DO
void pstmnt();              //Parse and Compile Program Statement
