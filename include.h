/*************************************
 * C02 Include File Parsing Routines *
 *************************************/

char line[255];     /*Entire line parsed from include file*/

void logdef();  //Print Definition Table to Log File
void pdefin();  //Process define directive
void pincfl();  //Process include file
void pprgma();  //Parse Pragma Directive
