/************************************
 * C02 Statement Compiling Routines *
 ************************************/

char asnvar[VARLEN+1];         //Assigned Variable Name
char asnidx[VARLEN+1] ;        //Assigned Variable Index
int  asnivt;                    //Assigned Index Variable Type

char xstmnt[LINELEN];          //Required Statement

void endblk();                 //End Program Block
void pstmnt();                 //Parse and Compile Program Statement
