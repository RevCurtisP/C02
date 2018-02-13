/*************************************
 * C02 Include File Parsing Routines *
 *************************************/

char line[255];     /*Entire line parsed from include file*/

void incasm();  //Process assembly language include file
void inchdr();  //Process header include file
void logdef();  //Print Definition Table to Log File
void pascii();  //Parse ASCII Subdirective
void pdefin();  //Process define directive
void phdwrd();  //Parse Header Word
void pincdr();  //Process Include File Directive
void pincfl();  //Process include file
void pincnm();  //Parse Include File Name  
void porign();  //Parse Origin Subdirective
void pprgma();  //Parse Pragma Directive
void pvrtbl();  //Process Vartable Subdirective
void prszpg();  //Parse Zeropage Subdirective
void rstsrc();  //Restore Source File Pointer
void savsrc();  //Save Source File Information
void setinc();  //Set Include File Information
