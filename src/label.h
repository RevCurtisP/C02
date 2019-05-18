/******************************************************
 * C02 Label Parsing, Generation, and Lookup Routines *
 ******************************************************/

/* Symbol Types */
enum symtyps {SYMREG=1, SYMLBL=2, SYMCON=4, SYMVAR=8, SYMFNC=16, CONVAR=128};

struct symstc {
  char name[SYMLEN]; //Symbol Name
  int  type;         //Symbol Type
};

struct symstc symbol;              //Symbol Record
struct symstc symtbl[MAXSYM];      //Symbol Table
int  symcnt;					   //Number of Symbols

char curlbl[LABLEN+1];             //Most recently generated label
char cmplbl[LABLEN+1];             //Label for Comparison
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

enum ltypes {LTNONE, LTIF, LTELSE, LTLOOP, LTEND, LTDO, LTDWHL, LTSLCT, LTCASE, LTFUNC}; //Label Types
enum lflags {LFNONE, LFBGN, LFEND}; //Label Flag Types

void addsym(char *name, int type); //Add Symbol to Table
int  fndsym(char *name);		   //Find Symbol in Table

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

