/************************************
 * C02 Declaration Compiling Routines *
 ************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "common.h"
#include "asm.h"
#include "parse.h"
#include "label.h"
#include "vars.h"
#include "cond.h"
#include "expr.h"
#include "stmnt.h"
#include "dclrtn.h"

/* Add Function Definition */
void addfnc(void) {
  expect('(');
  strcpy(fncnam, word);   //Save Function Name
  prmcnt = 0;             //Set Number of Parameters
  skpspc();               //Skip Spaces
  if (isalph()) {         //Parse Parameters
    reqvar(FALSE);        //Get First Parameter
    strcpy(prmtra, value);
    prmcnt++;     
    if (look(',')) {
      reqvar(FALSE);      //Get Second Parameter
      strcpy(prmtry, value);
      prmcnt++;     
      if (look(',')) {
        reqvar(FALSE);    //Third Parameter
        strcpy(prmtrx, value);
        prmcnt++;     
      }
    }
  }
  expect(')');
  if (look(';'))          //Forward Definition
    return;
	  setlbl(fncnam);         //Set Function Entry Point
  if (prmcnt > 0)       
    asmlin("STA", prmtra); //Store First Parameter
  if (prmcnt > 1)
    asmlin("STY", prmtry); //Store Second Parameter
  if (prmcnt > 2)
    asmlin("STX", prmtrx); //Store Third Parameter
  endlbl[0] = 0;          //Create Dummy End Label
  pshlbl(LTFUNC, endlbl); //and Push onto Stack
  bgnblk('{');           //Start Program Block
}

/* (Check For and) Parse Variable Declaration*/
void pdecl(int m, int t) {
  DEBUG("Processing variable declarations(s) of type %d\n", t);
  while(TRUE) {
    getwrd();
    if (match('(')) {
      if (m != MTNONE)
        ERROR("Illegal Modifier %d in Function Definion", m, EXIT_FAILURE)
      
      addfnc();  //Add Function Call
      return;
    }  
    addvar(m, t);
    if (!look(','))
      break;
  }    
  expect(';');
  DEBUG("Variable Declaration Completed\n", 0);
  SCMNT("");  //Clear Assembler Comment
}

/* Check for and Parse Type Keyword */
int ptype(int m) {
  int result = TRUE;
  if (wordis("VOID"))
    pdecl(m, VTVOID);   //Parse 'void' declaration
  else if (wordis("CHAR"))
    pdecl(m, VTCHAR);   //Parse 'char' declaration
  else
    result = FALSE;
  //DEBUG("Returning %d from function ptype\n", result)'
  return result;
}

/* Check for and Parse Modifier */
int pmodfr(void) {
  DEBUG("Parsing modifier '%s'\n", word);
  int result = TRUE;
  if (wordis("ALIGNED")) {
    getwrd();
    ptype(MTALGN);  
  }
  else if (wordis("ZEROPAGE")) {
    getwrd();
    ptype(MTZP);  
  }
  else
    result = FALSE;
  return result;
}
