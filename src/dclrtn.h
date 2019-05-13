/************************************
 * C02 Declaration Compiling Routines *
 ************************************/

char fncnam[VARLEN+1]; //Function Name
char prmtra[VARLEN+1]; //Function Parameter A
char prmtrx[VARLEN+1]; //Function Parameter X
char prmtry[VARLEN+3]; //Function Parameter Y
int  prmcnt;           //Number of Parameters
//int  lpemtd;                    //Location Prefix Emitted

void addcon(int numval); //Add Constant
int pmodfr();            //Check for and Parse Modifier
int ctype(int reqtyp);	 //Check for Type Keyword
int ptype(int m);        //Check for and Parse Type Keyword

enum types {TNONE, TVOID, TENUM, TBITMASK,TCHAR, TINT, TSTRUCT};
