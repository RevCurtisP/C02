/*************************************
 * C02 Common Definitions & Routines *
 *************************************/

#define FNAMLEN 255       //Maximum File Name Length
#define LINELEN 255       //Maximum Input/Output Line Length
#define CONLEN 6          //Maximum Constant Name Length
#define MAXCON 255        //Maximum Number of Constants
#define STCLEN 6          //Maximum Struct Name Length
#define MAXSTC 32         //Maximum Number of Stuctures
#define STMLEN 6          //Maximum Struct Member Name Length
#define MAXSTM 255        //Maximum Number of Stucture Members
#define VARLEN 6          //Maximum Variable Name Length
#define MAXVAR 255        //Maximum Number of Variables
#define MAXTRM 16         //Maximum Terms in Stack
#define DATASPC 4096      //Space to Allocate for Variable Data
#define SUBMAX 4          //Maximum Number of Sub Directories

#define LABLEN 6          //Maximum Program Label Length
#define MAXLAB 255        //Maximum Number of Program Labels

#define LBLLEN 6          //Maximum Label Length
#define LBLFMT "L_%04d"   //Label Format
#define LABSFX ":"        //Label Suffix
#define MAXLBL 15         //Maximum Number of Labels (Nesting Depth)
#define LOCPFX "."        //Local Variable Prefix

#define CPUOP  "PROCESSOR"  //Target CPU Pseudo-Operator 
#define CPUARG "6502"       //Target CPU Operand
#define ORGOP  "ORG"        //Origin Pseudo-Op
#define EQUOP  "EQU"        //Equate Pseudo-Op
#define BYTEOP "BYTE"       //Define Byte Pseudo-Op
#define STROP  "DS"         //Define String Pseudo-Op
#define ALNOP  "ALIGN"      //Align Pseudo-Op
#define USEGOP "SEG.U"      //Uninitalized Segment Pseudo-Op
#define LOCOP  "SUBROUTINE" //Local Variable Boundary Pseudo-Op
#define LOCEND "ENDSUB"		//End Local Variables Pseudo-Op

#define ASMFMT "%-7s %-3s %-12s %s\n"  //Assembly Language Line printf Format

/* Internal Constants and Macros */
#define TRUE -1
#define FALSE 0
#define NOT(exp) (exp) == 0
#define TF(x) (x) ? TRUE : FALSE;


void prtpos(); //Print current file name and position
#define DEBUG(fmt, val) {if (debug) {prtpos(); printf(fmt, val);}}
#define DETAIL(fmt, val) {if (debug) printf(fmt, val);}
#define ERROR(fmt, val, err) {fprintf(stderr, fmt, val);exterr(err);}

int debug;  //Print Debug Info (TRUE or FALSE)
int cmos;   //Flag: Use 65C02 Instruction Set

int  gencmt;           //Generate Assembly Language Comments
char asmcmt[LINELEN];  //Processed Assembly Language Comment

int curcol, curlin;  //Position in Source Code
int savcol, savlin;  //Save Position in Source Code

int bgntim; //Starting Time

int nxtchr; //Next Character of Source File to Process
int nxtupc; //Next Character Converted to Uppercase
int savchr; //Holds nxtchr when switching input files

int  wrdlen;          //Length of Parsed Word
char word[LINELEN];   //Word parsed from source file
char uword[LINELEN];  //Word converted to uppercase
int  pstlen;          //Length of Parsed String
char pstrng[LINELEN]; //String parsed fron source file
char cmtasm[LINELEN]; //Assembly Language Comment Text
char cputyp[LINELEN]; //CPU Type

char hdrnam[FNAMLEN]; //Header File Name
char incdir[FNAMLEN]; //Include File Directory
char inpnam[FNAMLEN]; //Input File Name 
char subdir[SUBMAX][FNAMLEN]; //Include File SubDirectory
int  subcnt; //Number of Include Directories
int  subidx; //Index into subdir[]

int alcvar; //Allocate Variables Flag
int inblck; //Multiline Block Flag
int infunc; //Inside Function Definition Flag
int lsrtrn; //Last Statement was a Return Flag
int fcase;  //First Case Statement Flag

int padcnt;	//Number of Padding Bytes at End of Program

void exterr(int errnum);        //Print current file name & position and exit
void expctd(char *expected);    //Print Expected message and exit

void addcmt(char *s);   //Append string to comment
void chrcmt(char c);    //Append character to comment
void setcmt(char *s);   //Set comment to string
#define SCMNT(str) if (gencmt) {setcmt(str);}
#define ACMNT(str) if (gencmt) {addcmt(str);}
#define CCMNT(chr) if (gencmt) {chrcmt(chr);}
#define LCMNT(str) if (gencmt) {setcmt(str); cmtlin();}
