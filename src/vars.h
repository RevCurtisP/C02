/*************************************
 * C02 Variable Management Routines  *
 *************************************/

/* Variable Record */
struct varrec {
  char name[VARLEN+1]; //Variable Name Table (string)
  char modifr;         //Variable Modifier (MTxxx)
  char type;           //Variable Type (VTxxx)
  char size[4];        //Variable Array Size (string)
  char stcidx;         //Variable Struct Index
};

struct varrec vartbl[MAXVAR+1];	//Variable Table
struct varrec varble;		    //Variable Table Entry

int  varcnt;                     //Number of Variables in Table
int  varidx;                     //Index into Variable Tables
char vrname[MAXVAR+1];           //Variable Name
int  vartyp;	 				 //Variable/Member Type
int  vrwrtn;                     //Variables Written Flag

struct strctd {                  //Struct Definition
  char name[STCLEN+1];           //Struct Name
  int  size;                     //Total Length of Struct
};                               

struct strctd strcts[MAXSTC+1];  //Table of Defined Structs
struct strctd strct;             //Defined Struct
int  stccnt;                     //Number of Structs Defined
int  stcidx;                     //Index into Struct Tables

struct membrd {                  //Struct Membetr Definition
  char name[STMLEN+1];           //Member Name
  int  strcti;                   //Parent Struct Index
  int  vartyp;                   //Member Symbol Type
  int  symidx;					 //Member Symbol Index
  int  offset;                   //Offset into Struct
  int  size;                     //Member Size
};

struct membrd membrs[MAXSTM+1];  //Table of Defined Members
struct membrd membr;             //Defined Member
int mbrcnt;                      //Number of Struct Members Defined
int mbridx;                      //Index into Struct Member Tables
int mbrofs;						 //Member Offset
int mbrsiz;                      //Member Size

enum vtypes {VTVOID, VTREG, VTCHAR, VTINT, VTARRAY, VTSTRUCT}; //Variable Types

char datvar[DATASPC+1];        //Variable Data Storage
int  datlen[MAXVAR+1];         //Variable Data Length
char dattyp[MAXVAR+1];         //Variable Data Type
char dattmp[DATASPC+1];        //Variable Temporary Data
int  dtype;                    //Data Type
int  dlen;                     //Length of Variable Data
int  dsize;                    //Total Data Length

enum dtypes {DTBYTE, DTINT, DTSTR, DTARRY};  //Variable Data Types

/* Variable Modifier Types (Bit Mask) */
#define MTNONE   0 //No Modifier
#define MTALS    1 //Alias
#define MTCONST  2 //Constant
#define MTZP     4 //Zero Page
#define MTALGN 128 //Aligned

/*Value Parsing Flags (BitMask) */
#define ALWDEF	0  //Allow Default
#define ALWREG	1  //Allow Register
#define ALWCON  2  //Allow CONST Variable
#define ALWINT  4  //Allow Integer

int symdef(char *name);         //Is Variable defined (TRUE or FALSE)
int rambas;						//RAM Base Address (0=None)
int wrtbas;						//Write Base Address (0=None)
int zpaddr;                     //Current Zero-Page Address
char wrtofs[6];				    //Write Address Offset

void addvar(int m, int t);           //Parse and Compile Variable Declaration
void addstc();                       //Parse and Compile Structure Declaration
void defstc();                       //Parse Structure Definition
void chksym(int alwreg, int alwcon, char *name); //Error if Variable not defined
void prsdts();                       //Parse Data String
void setdat();                       //Set Variable Data
void setvar(int m, int t);           //Set Variable Name and Size
void prsdts();                       //Parse Data String
void prsvar(int alwreg, int alwcon); //Parse Variable
void prsmbr(char* name);             //Parse Struct Member
int  psizof(void);                   //Parse SizeOf Operator
int  pidxof(void);                   //Parse IndexOf Operator
void reqvar(int alwary);             //Require and Parse Variable Name
void setdat();                       //Store variable data
void setvar(int m, int t);           //Add Variable to Variable table
void wvrtbl();                       //Create Variable Table
void logstc(void);                   //Print Struct Tables to Log File
