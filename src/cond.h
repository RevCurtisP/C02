/************************************
 * C02 Conditional Parsing Routines *
 ************************************/

enum LOGOPS {LOPNONE, LOPAND, LOPOR};

int revcmp; //Reverse Comparison
int logopr; //Logical Operator (set to LOGOPS) 

void prscnd(char trmntr, int revrse); //Parse Conditional Expression 
