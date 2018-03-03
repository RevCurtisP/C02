/**********************************
 * C02 Expession Parsing Routines *
 **********************************/

char term[255]; //Term parsed from equation

char fnstck[MAXFNS][VARLEN+1];   //Function Call Stack
int  fnscnt;                     //Number of Functions in Stack

int  chkadr(int adract);    //Check for and Process Address or String
void chkidx();              //Check for, Parse, and Process Index
void prsadr(int adract);    //Parse and Compile Address of Operator
void prsfnc(char trmntr);   //Parse function call
void prsidx();              //Parse Array Index
void prstrm();              //Parse Term in Expression
void prsxpr(char trmntr);   //Parse Expression


