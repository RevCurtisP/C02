/*************************************
 * C02 Variable Management Routines  *
 *************************************/

/* Variable Table */
char varnam[MAXVAR+1][VARLEN+1];   //Variable Name
char vartyp[MAXVAR+1];             //Variable Type
char varsiz[MAXVAR+1][4];          //Variable Array
int  varcnt;                       //Number of Variables in Table

enum vtypes {VTVOID, VTBYTE, VTCHAR};    //Variable Types

char datvar[DATASPC+1];            //Variable Data Storage
char datlen[MAXVAR+1];             //Variable Data Length
char dattyp[MAXVAR+1];             //Variable Data Type
int  dtype;                    //Data Type
int  dlen;                     //Length of Variable Data
int  dsize;                    //Total Data Length

enum dtypes {DTBYTE, DTSTR};       //Variable Data Types

int symdef(char *name);   //Is Variable defined (TRUE or FALSE)
void chksym(char *name);  //Error if Variable not defined

void pdecl(int t); //Parse Variable Declaration

void vartbl();      //Create Variable Table
