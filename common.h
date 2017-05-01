/*************************************
 * C02 Common Definitions & Routines *
 *************************************/

#define FNAMLEN 255       //Maximum File Name Length
#define LINELEN 255       //Maximum Input/Output Line Length
#define DEFLEN 9          //Maximum Definition Text Length
#define MAXDEF 255        //Maximum Number of Definitions
#define VARLEN 6          //Maximum Variable Length
#define MAXVAR 255        //Maximum Number of Variables
#define MAXFNS 16         //Maximum Functions in Stack
#define DATASPC 2048      //Space to Allocate for Variable Data
#define LABLEN 6          //Maximum Label Length
#define LABFMT "L_%04d"   //Label Format
#define LABSFX ":"        //Label Suffix
#define MAXLAB 15         //Maximum Number of Labels (Nesting Depth)

#define CPUOP  "PROCESSOR" //Target CPU Pseudo-Operator 
#define CPUARG "6502"      //Target CPU Operand
#define EQUOP  "EQU"       //Equate Pseudo-Op
#define BYTEOP "DC"        //Define Byte Pseudo-Op
#define STROP  "DS"        //Define String Pseudo-Op

#define ASMFMT "%-7s %-3s %-12s %s\n"  //Assembly Language Line printf Format

/* Internal defines */
#define TRUE -1
#define FALSE 0

#define DEBUG(fmt, val) if (debug) {prtpos(); printf(fmt, val);}
#define ERROR(fmt, val, err) if (debug) {printf(fmt, val);exterr(err);}
#define SCMNT(str) if (gencmt) {setcmt(str);}
#define ACMNT(str) if (gencmt) {addcmt(str);}
#define CCMNT(chr) if (gencmt) {chrcmt(chr);}

int gencmt;        //Generate Assembly Language Comments
int debug;         //Print Debug Info (TRUE or FALSE)

int curcol, curlin;  //Position in Source Code
int savcol, savlin;  //Save Position in Source Code

int nxtchr; //Next Character of Source File to Process
int nxtupc; //Next Character Converted to Uppercase
int savchr; //Holds nxtchr when switching input files

char inpnam[FNAMLEN]; //Include File Name 

int alcvar; //Allocate Variables Flag

void exterr(int errnum); //Print current file name & position and exit
void expctd(char *expected); //Print Expected message and exit

void prtpos(); //Print current file name and position

