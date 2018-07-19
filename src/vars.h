/*************************************
 * C02 Variable Management Routines  *
 *************************************/

/* Variable Table */
char varnam[MAXVAR+1][VARLEN+1]; //Variable Name Table
char varmod[MAXVAR+1];           //Variable Modifier
char vartyp[MAXVAR+1];           //Variable Type
char varsiz[MAXVAR+1][4];        //Variable Array Size
char varstc[MAXVAR+1];           //Variable Struct Index
int  varcnt;                     //Number of Variables in Table
int  varidx;                     //Index into Variable Tables
char vrname[MAXVAR+1];           //Variable Name
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
  int  stype;                    //Member Symbol Type
  int  offset;                   //Offset into Struct
  int  size;                     //Member Size
};

struct membrd membrs[MAXSTM+1];  //Table of Defined Members
struct membrd membr;             //Defined Member
int mbrcnt;                      //Number of Struct Members Defined
int mbridx;                      //Index into Struct Member Tables

enum vtypes {VTVOID, VTCHAR, VTSTRUCT}; //Variable Types

char datvar[DATASPC+1];        //Variable Data Storage
char datlen[MAXVAR+1];         //Variable Data Length
char dattyp[MAXVAR+1];         //Variable Data Type
char dattmp[256];              //Variable Temporary Data
int  dtype;                    //Data Type
int  dlen;                     //Length of Variable Data
int  dsize;                    //Total Data Length

enum dtypes {DTBYTE, DTSTR, DTARRY};  //Variable Data Types

enum mtypes {MTNONE, MTALGN, MTZP, MTALS}; //Variable Modifier Types

int symdef(char *name);         //Is Variable defined (TRUE or FALSE)
int zpaddr;                     //Current Zero-Page Address

char fncnam[VARLEN+1];          //Function Name
char prmtra[VARLEN+1];          //Function Parameter A
char prmtrx[VARLEN+1];          //Function Parameter X
char prmtry[VARLEN+1];          //Function Parameter Y
int  prmcnt;                    //Number of Parameters

void addvar(int m, int t);           //Parse and Compile Variable Declaration
void addstc();                       //Parse and Compile Structure Declaration
void defstc();                       //Parse Structure Definition
void chksym(int alwreg, char *name); //Error if Variable not defined
void prsdts();                       //Parse Data String
void setdat();                       //Set Variable Data
void setvar(int m, int t);           //Set Variable Name and Size
void prsdts();                       //Parse Data String
void prsvar(int alwreg);             //Parse Variable
void prsmbr(char* name);             //Parse Struct Member
int  psizof(void);                   //Parse SizeOf Operator
void reqvar(int alwary);             //Require and Parse Variable Name
void setdat();                       //Store variable data
void setvar(int m, int t);           //Add Variable to Variable table
void vartbl();                       //Create Variable Table
void logstc(void);                   //Print Struct Tables to Log File
