/******************************************************
 * C02 Label Parsing, Generation, and Lookup Routines *
 ******************************************************/

int  inblck;                       //Multiline Block Flag

char curlbl[LABLEN+1];             //Most recently generated label
char cndlbl[LABLEN+1];             //Label for Conditional Code
char endlbl[LABLEN+1];             //End Label
char forlbl[LABLEN+1];             //For Loop Label
char loplbl[LABLEN+1];             //Skip Increment Label
char skplbl[LABLEN+1];             //Skip Increment Label
char tmplbl[LABLEN+1];             //Temporary Label
char lblnam[MAXLAB+1][LABLEN+1];   //Label Name Table
int  lbltyp[MAXLAB+1];             //Label Type
int  lblblk[MAXLAB+1];             //Label Ends Program Block
int  lblcnt;                       //Number of Labels in stack
int  lblnxt;                       //Sequence of next label to be generated
char lbltmp[LABLEN+1];             //Label Temporary Storage

enum ltypes {LTNONE, LTIF, LTLOOP, LTEND, LTDO, LTDWHL, LTSLCT, LTCASE, LTFUNC}; //Label Types
enum lflags {LFNONE, LFBGN, LFEND}; //Label Flag Types

void prslbl();              //Parse Label From Code
void newlbl(char* lbname);  //Generate New Block Label
int  poplbl();              //Pull Last Label and Emit on Next Line
void setlbl();              //Emit word as Label on Next Line

