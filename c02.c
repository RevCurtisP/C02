/**************************************************************
 * C02 Compiler - (C) 2013 Curtis F Kaylor                    *
 *                                                            *
 * C02 is a modified C-like language designed for the 6502    *
 *                                                            * 
 * This Compiler generates crasm compatible assembly language *
 *                                                            *
 **************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "common.h"  //Common Code used by all Modules
#include "files.h"   //Open and Close Files
#include "asm.h"     //Write out Assembly Language
#include "parse.h"   //General Code Parsing
#include "vars.h"    //Variable Parsing, Lookup, and Allocation
#include "expr.h"    //Expression Parsing
#include "label.h"   //Label Parsing, Generation, and Lookup
#include "cond.h"    //Conditional Parsing
#include "stmnt.h"   //Statement Compiling Code
#include "include.h" //Include File Parsing

/* Initilize Compiler Variables */
void init()
{
  DEBUG("Initializing Compiler Variables\n",0);
  defcnt = 0;
  varcnt = 0;
  lblcnt = 0;
  cmpsgn = 0;
  curcol = 0;
  curlin = 0;
  inpfil = srcfil;
  strcpy(inpnam, srcnam);
  alcvar = TRUE;
  nxtwrd[0] = 0;
  nxtptr = 0;
}

/* Reads and parses the next Word in Source File */
pword()
{
  getwrd();
  SCMNT(word);
  DEBUG("Parsing Word '%s'\n", word);
  if (wordis("byte")) 
    pdecl(VTBYTE);   //Parse 'byte' declaration
  else if (wordis("char")) 
    pdecl(VTCHAR);   //Parse 'char' declaration
  else 
    pstmnt();     //Parse Statement
}

/* Process a directive */
void pdrctv()
{
  skpchr();            //skip '#'
  getwrd();           //read directive into word
  DEBUG("Processing directive '%s'\n", word);
  if (wordis("define"))
    prsdef();  //Parse Define
  if (wordis("include"))
    pincfl();  //Parse Include File
  else if (wordis("error"))
    ERROR("Error \n", 0, EXIT_FAILURE);
}

void epilog()
{
  vartbl();  //Write Variable Table
}

/* Compile Source Code*/
void compile()
{
  DEBUG("Starting Compilation\n", 0);
  init();
  prolog();
  skpchr();
  DEBUG("Parsing Code\n", 0);
  while (TRUE)  
  {
    skpspc();
    //DEBUG("Checking next character '%c'\n", nxtchr);
    if (match(EOF)) 
      break;
    else if (match('#'))
      pdrctv(); //Process Directive
    else if (match('/')) 
      skpcmt();
    else if (isalph()) 
      pword();
    else
	  {
      printf("Unexpected character '%c'\n", nxtchr);	 
		  exterr(EXIT_FAILURE);
	  }
  }    
  epilog();
}

/* Display "Usage" text and exit*/ 
void usage()
{
  printf("Usage: c02 sourcefile.c02\n");
  exit(EXIT_FAILURE);      
}

/* Parse Command Line Arguments                                 *   
 *  Sets: srcnam - Source File Name (from first arg)           *
 *        outnam - Output File Name (from optional second arg) */
void pargs(int argc, char *argv[])
{ 
 DEBUG("Parsing %d Arguments\n", argc);
 if (argc < 2) usage(); //at least one argument is required
 strcpy(srcnam, argv[1]); //set Source File Name to first arg
 DEBUG("srcnam set to '%s'\n", srcnam);
 if (argc > 2) //if second argument exists
   strcpy(outnam, argv[2]); //set Out File Name to second arg
 else strcpy(outnam, ""); //else set to null string
 DEBUG("outnam set to '%s'\n", outnam);
}

int main(int argc, char *argv[])
{
  debug = TRUE;  //Output Debug Info
  gencmt = TRUE; //Generate Assembly Language Comments
  
  printf("C02 Compiler (C) 2012 Curtis F Kaylor\n" );
  
  pargs(argc, argv); //Parse Command Line Arguments
  
  opnsrc();  //Open Source File
  opnout();  //Open Output File
  opnlog();  //Open Log File

  compile();

  logvar();

  clssrc();  //Close Source File
  clsout();  //Close Output File
  clslog();  //Close Log File
}


