/*************************************
 * C02 Include File Parsing Routines *
 *************************************/

char line[255];     /*Entire line parsed from include file*/

void logcon();  //Print Constant Table to Log File
void pdefin();  //Process define directive
void pdefin();  //Process define directive
void penumd();  //Process enum directive
void phdrfl();  //Process command line header file
void pincfl();  //Process include file
void pprgma();  //Parse Pragma Directive
void setsrc();  ///Set Input to Source File
