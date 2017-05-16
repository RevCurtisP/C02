/**********************************
 * C02 Expession Parsing Routines *
 **********************************/

char term[255];   /*Term parsed from equation*/
int  trmtxt;      /*Term Type*/

char fnstck[MAXFNS][VARLEN+1];   //Function Call Stack
int  fnscnt;                     //Number of Functions in Stack

void prsidx();            //Parse Array Index
void prstrm();            //Parse Term in Expression
void prsxpr(char trmntr); //Parse Expression


