/**************************************************************
 * C02 Compiler - (C) 2013 Curtis F Kaylor                    *
 *                                                            *
 * C02 is a simpified C-like language designed for the 6502   *
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
  inblck = FALSE;
  xstmnt[0] = 0;
  nxtwrd[0] = 0;
  nxtptr = 0;
  vrwrtn = FALSE;
  zpaddr = 0;
}



/* Reads and parses the next Word in Source File */
pword()
{
  lsrtrn = FALSE; //Clear RETURN flag
  getwrd();
  ACMNT(word);
  DEBUG("Parsing Word '%s'\n", word);
  if (xstmnt[0])
    if (wordis(xstmnt))
      xstmnt[0] = 0;
    else
      ERROR("Expected '%s' statement\n", xstmnt, EXIT_FAILURE);
  if (!pmodfr() && !ptype(MTNONE))
    pstmnt();     //Parse Statement
}

/* Process a directive */
void pdrctv()
{
  skpchr();            //skip '#'
  getwrd();           //read directive into word
  DEBUG("Processing directive '%s'\n", word);
  if (wordis("DEFINE"))
    pdefin();  //Parse Define
  else if (wordis("INCLUDE"))
    pincfl();  //Parse Include File
  else if (wordis("ERROR")) {
    ERROR("Error \n", 0, EXIT_FAILURE);
  }
  else if (wordis("PRAGMA"))
    pprgma();
  else
    ERROR("Illegal directive %s encountered\n", word, EXIT_FAILURE);
}

void epilog()
{
  if (!vrwrtn) vartbl();  //Write Variable Table
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
    else if (match('}'))
      endblk(TRUE);  //End Multi-Line Program Block
    else if (match('#'))
      pdrctv();      //Parse Directive
    else if (match('/')) 
      skpcmt();      //Skip Comment
    else if (isalph()) 
      pword();       //Parse Word
    else
      ERROR("Unexpected character '%c'\n", nxtchr, EXIT_FAILURE);
  }    
  epilog();
}

/* Display "Usage" text and exit*/ 
void usage()
{
  printf("Usage: c02 sourcefile.c02\n");
  exit(EXIT_FAILURE);      
}

/* Parse Command Line Argument */
int popt(int arg, int argc, char *argv[])
{
  char opt; //Option
  char optarg[32]; //Option Argument
  opt = argv[arg][1];
  //if strchr(opt, "i") {
  //if (strlen(argv[arg] > 2)
  //}
  ERROR("Illegal option -%c\n", opt, EXIT_FAILURE);
}

/* Parse Command Line Arguments                                 *   
 *  Sets: srcnam - Source File Name (from first arg)           *
 *        outnam - Output File Name (from optional second arg) */
void pargs(int argc, char *argv[])
{ 
  int arg;
  srcnam[0] = 0;
  outnam[0] = 0;
  DEBUG("Parsing %d arguments\n", argc);
  if (argc == 0) usage(); //at least one argument is required
  for (arg = 1; arg<argc; arg++) {
    DEBUG("Parsing argument %d\n", arg);
    if (argv[arg][0] == '-') {
      arg = popt(arg, argc, argv);
    }
    else if (srcnam[0] == 0) {
      strcpy(srcnam, argv[arg]); //set Source File Name to first arg
      DEBUG("srcnam set to '%s'\n", srcnam);
    }
    else if (outnam[0] == 0) {
       strcpy(outnam, argv[arg]); //set Out File Name to second arg
       DEBUG("outnam set to '%s'\n", outnam);
    }
    else
      ERROR("Unexpected argument '%s'\n", argv[arg], EXIT_FAILURE);
  }
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

  logdef();
  logvar();

  clssrc();  //Close Source File
  clsout();  //Close Output File
  clslog();  //Close Log File
}


