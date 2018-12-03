/**********************************
 * C02 Expession Parsing Routines *
 **********************************/

enum adacts {ADNONE, ADLDYX, ADPUSH};

char term[255]; //Term parsed from equation

char oprstk[MAXTRM];	         //Operator Stack
char trmstk[MAXTRM][VARLEN+1];   //Function/Index Terms Stack
int  trmidx;                     //Next Index in Stack

int  trmcnt;  //Number of total terms in current expression

int  chkadr(int adract, int alwstr);  //Check for and Process Address or String
void chkidx();              //Check for, Parse, and Process Index
void prcftm();              //Process First Term
void prsadr(int adract);    //Parse and Compile Address of Operator
void prsfnc(char trmntr);   //Parse function call
void prsidx();              //Parse Array Index
void prstrm();              //Parse Term in Expression
void prsrxp(char trmntr);    //Parse Rest of Expression 
void prsxpr(char trmntr);   //Parse Expression


