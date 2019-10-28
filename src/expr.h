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
int  prcftm(int alwint);    //Process First Term
void prcvri(void);			//Process Integer Variable
int  prcivr(int alwint);    //Process Integer Variable in Term
void prsadr(int adract);    //Parse and Compile Address of Operator
void prsbop(void);			//Parse Byte Operator
void prsval(int alwreg, int alwcon); //Parse Value
void prsfnc(char trmntr);   //Parse function call
void prsfpr(char trmntr);   //Parse Function Paraeters or Return
void prsidx();              //Parse Array Index
int  prstrm(int alwint);    //Parse Term in Expression
void prsrxp(char trmntr);   //Parse Rest of Expression 
int  prsxpf(char trmntr);   //Parse Expression in Function Call
void prsxpr(char trmntr);   //Parse Expression
void prsxpi(char trmntr, int asmxpr); //Parse Integer Expression
