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
#define DATASPC 2048      //Space to Allocate for Variable Data

#define LABLEN 6          //Maximum Label Length
#define LABFMT "L_%04d"   //Label Format
#define LABSFX ":"        //Label Suffix
#define MAXLAB 15         //Maximum Number of Labels (Nesting Depth)
#define LOCPFX "."        //Local Variable Prefix

#define CPUOP  "PROCESSOR"  //Target CPU Pseudo-Operator 
#define CPUARG "6502"       //Target CPU Operand
#define ORGOP  "ORG"        //Origin Pseudo-Op
#define EQUOP  "EQU"        //Equate Pseudo-Op
#define BYTEOP "DC"         //Define Byte Pseudo-Op
#define STROP  "DS"         //Define String Pseudo-Op
#define ALNOP  "ALIGN"      //Align Pseudo-Op
#define LOCOP  "SUBROUTINE" //Local Variable Boundary Pseudo-Op

#define ASMFMT "%-7s %-3s %-12s %s\n"  //Assembly Language Line printf Format

/* Internal defines */
#define TRUE -1
#define FALSE 0

void initim(); //Initialize elapsed time counter
void prtpos(); //Print current file name and position
void prttim(); //Print elapsed time
#define DEBUG(fmt, val) {if (debug) {prtpos(); prttim(); printf(fmt, val);}}
#define DETAIL(fmt, val) {if (debug) printf(fmt, val);}
#define ERROR(fmt, val, err) {fprintf(stderr, fmt, val);exterr(err);}

int debug;  //Print Debug Info (TRUE or FALSE)

int  gencmt;           //Generate Assembly Language Comments
char asmcmt[LINELEN];  //Processed Assembly Language Comment

int curcol, curlin;  //Position in Source Code
int savcol, savlin;  //Save Position in Source Code

int bgntim; //Starting Time

int nxtchr; //Next Character of Source File to Process
int nxtupc; //Next Character Converted to Uppercase
int savchr; //Holds nxtchr when switching input files

char word[LINELEN];   //Word parsed from source file
char uword[LINELEN];  //Word converted too uppercase
char cmtasm[LINELEN]; //Assembly Language Comment Text

char hdrnam[FNAMLEN]; //Header File Name
char incdir[FNAMLEN]; //Include File Directory
char inpnam[FNAMLEN]; //Input File Name 

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
