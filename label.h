/******************************************************
 * C02 Label Parsing, Generation, and Lookup Routines *
 ******************************************************/

int  inblck;                       //Multiline Block Flag

char curlbl[LABLEN+1];             //Most recently generated label
char endlbl[LABLEN+1];             //End Label
char loplbl[LABLEN+1];             //Skip Increment Label
char skplbl[LABLEN+1];             //Skip Increment Label
char tmplbl[LABLEN+1];             //Temporary Label
char lblnam[MAXLAB+1][LABLEN+1];   //Label Name Table
int  lbltyp[MAXLAB+1];             //Label Type
int  lblblk[MAXLAB+1];             //Label Ends Program Block
int  lblcnt;                       //Number of Labels in stack
int  lblnxt;                       //Sequence of next label to be generated
char lbltmp[LABLEN+1];             //Label Temporary Storage

enum ltypes {LTIF, LTLOOP, LTEND, LTDO, LTCASE}; //Label Types

int lstlbl(int lbtype);    //Find Last Label of Specified Type
void prslbl();              //Parse Label From Code
void newlbl(); //Generate New Block Label
int poplbl();               //Pull Last Label and Emit on Next Line
void setlbl();             //Emit word as Label on Next Line

