/******************************
 * C02 File Handling Routines *
 ******************************/

FILE *srcfil;  //Source File (Input)
FILE *outfil;  //Assembler File (Output)
FILE *logfil;  //Log File (Output) 
FILE *incfil;  //Include File Handle
FILE *inpfil;  //Current Input File

char srcnam[FNAMLEN]; //Source File Name
char outnam[FNAMLEN]; //Assembler File Name
char lognam[FNAMLEN]; //Log File Name 
char incnam[FNAMLEN]; //Include File Name 
char subnam[FNAMLEN]; //Include File Name (Subdirectory) 

void opnsrc();   //Open Source File
void clssrc();   //Close Source File

void opnout();   //Open Output File
void clsout();   //Close Output File

void opnlog();   //Open Log File
void clslog();   //Close Log File

void opninc();   //Open Include File
void clsinc();   //Close Include File
