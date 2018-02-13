/*************************************
 * C02 Variable Management Routines  *
 *************************************/

/* Variable Table */
char varnam[MAXVAR+1][VARLEN+1];   //Variable Name Table
char varmod[MAXVAR+1];             //Variable Modifier
char vartyp[MAXVAR+1];             //Variable Type
char varsiz[MAXVAR+1][4];          //Variable Array
int  varcnt;                       //Number of Variables in Table
int  varidx;                       //Index into Variable Tables
char vrname[MAXVAR+1];             //Variable Name
int  vrwrtn;                       //Variables Written Flag

/*
int varidx; //Index into Variable Table
int vrtype; //Variable Type
*/
enum vtypes {VTVOID, VTCHAR};  //Variable Types

char datvar[DATASPC+1];        //Variable Data Storage
char datlen[MAXVAR+1];         //Variable Data Length
char dattyp[MAXVAR+1];         //Variable Data Type
char dattmp[256];              //Variable Temporary Data
int  dtype;                    //Data Type
int  dlen;                     //Length of Variable Data
int  dsize;                    //Total Data Length

enum dtypes {DTBYTE, DTSTR, DTARRY};  //Variable Data Types

enum mtypes {MTNONE, MTALGN, MTZP}; //Variable Modifier Types

int symdef(char *name);         //Is Variable defined (TRUE or FALSE)
int zpaddr;                     //Current Zero-Page Address

char fncnam[VARLEN+1];          //Function Name
char prmtra[VARLEN+1];          //Function Parameter A
char prmtrx[VARLEN+1];          //Function Parameter X
char prmtry[VARLEN+1];          //Function Parameter Y
int  prmcnt;                    //Number of Parameters

void addvar(int m, int t);      //Parse and Compile Variable Declaration
void chksym(int alwreg, char *name); //Error if Variable not defined
void prsdts();                  //Parse Data String
void setdat();                  //Set Variable Data
void setvar(int m, int t);      //Set Variable Name and Size
void prsdts();                  //Parse Data String
void prsvar(int alwreg);        //Parse Variable
void reqvar(int alwary);        //Require and Parse Variable Name
void setdat();                  //Store variable data
void setvar(int m, int t);      //Add Variable to Variable table
void vartbl();                  //Create Variable Table
