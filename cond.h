/************************************
 * C02 Conditional Parsing Routines *
 ************************************/

int cmpsgn;  // Comparison contains signed operands

int enccmp(char c);         //Encode Comparison Operator Character
void prccmp();              //Process and Compile Comparison Operator
void prsflg(int revrse);    //Parse Flag Operator
int prscmp(int revrse);     //Parse Comparison
void prscnd(char trmntr, int revrse); //Parse Conditional Expression 
