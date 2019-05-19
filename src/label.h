/******************************************************
 * C02 Label Parsing, Generation, and Lookup Routines *
 ******************************************************/

char curlbl[LBLLEN+1];             //Most recently generated label
char cmplbl[LBLLEN+1];             //Label for Comparison
char cndlbl[LBLLEN+1];             //Label for Conditional Code
char endlbl[LBLLEN+1];             //End Label
char forlbl[LBLLEN+1];             //For Loop Label
char loplbl[LBLLEN+1];             //Skip Increment Label
char skplbl[LBLLEN+1];             //Skip Increment Label
char tmplbl[LBLLEN+1];             //Temporary Label
char lblnam[MAXLBL+1][LBLLEN+1];   //Label Name Table
int  lbltyp[MAXLBL+1];             //Label Type
int  lblblk[MAXLBL+1];             //Label Ends Program Block
int  lblcnt;                       //Number of Labels in stack
int  lblnxt;                       //Sequence of next label to be generated
char lbltmp[LBLLEN+1];             //Label Temporary Storage

enum ltypes {LTNONE, LTIF, LTELSE, LTLOOP, LTEND, LTDO, LTDWHL, LTSLCT, LTCASE, LTFUNC}; //Label Types
enum lflags {LFNONE, LFBGN, LFEND}; //Label Flag Types

void chklbl(char* lbname);  //Check Label Contents
int  lstlbl(int lbflag);    //Find Last Label of Specified Types *
void newlbl(char* lbname);  //Generate New Block Label
int  poplbl();              //Pop Last Label and Emit on Next Line
void prslbl();              //Parse Label From Code
void pshlbl(int lbtype, char* lbname);  //Push Label onto Stack
void reqlbl(char* lbname);  //Require Label
void setblk(int blkflg);    //Set Block Flag for Last Label
void setlbl(char *lblset);  //Emit word as Label on Next Line
int  toplbl(char *rtlbl);   //Get Top Label and Return Type

