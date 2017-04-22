/******************************************************
 * C02 Label Parsing, Generation, and Lookup Routines *
 ******************************************************/

char curlbl[LABLEN+1];             //Most recently generated label
char endlbl[LABLEN+1];             //Label at end of block
char lblnam[MAXLAB+1][LABLEN+1];   //Label Name Table
int  lbltyp[MAXLAB+1];             //Label Type
int  lblcnt;                       //Number of Labels in stack
int  lblnxt;                       //Sequence of next label to be generated
char lbltmp[LABLEN+1];             //Label Temporary Storage

enum ltypes {LTIF, LTLOOP, LTEND}; //Label Types

void prslbl();     //Parse Label From Code
void newlbl();     //Generate New Block Label
void lbllin();     //Pull Last Label and Emit on Next Line
void setlbl();     //Emit word as Label on Next Line

