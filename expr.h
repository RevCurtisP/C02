/**********************************
 * C02 Expession Parsing Routines *
 **********************************/

char term[255]; //Term parsed from equation

char fnstck[MAXFNS][VARLEN+1];   //Function Call Stack
int  fnscnt;                     //Number of Functions in Stack

int chkpst;     //Check for Post-Operator

int chkadr(int adract);      //Check for and Process Address or String
void chkidx();              //Check for, Parse, and Process Index
void prcadr(int adract, char* symbol);  //Process Address Reference
void prcopr();              //Process Arithmetic or Bitwise Operator
void prsadr(int adract);    //Parse and Compile Address of Operator
void prsfnc(char trmntr);   //Parse function call
void prsftm();              //Parse first term of expession
void prsidx();              //Parse Array Index
void prstrm();              //Parse Term in Expression
void prsstr(int adract);    //Parse and Create Anonymous String
void prsval(int alwreg);    //Parse value (constant or identifier)
void prsxpr(char trmntr);   //Parse Expression


