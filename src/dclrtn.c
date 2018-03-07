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
  /* QUESTION: Eliminate this in favor of Register Assignments inside Function Call? */
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
  if (look(';')) return;    //Forward Definition
  setlbl(fncnam);         //Set Function Entry Point
  if (prmcnt > 0) asmlin("STA", prmtra); //Store First Parameter
  if (prmcnt > 1) asmlin("STY", prmtry); //Store Second Parameter
  if (prmcnt > 2) asmlin("STX", prmtrx); //Store Third Parameter
  endlbl[0] = 0;          //Create Dummy End Label
  pshlbl(LTFUNC, endlbl); //and Push onto Stack
  bgnblk('{');           //Start Program Block
}

/* Parse Constant Declaration*/
void pconst(int m) {
  DEBUG("Processing constant declarations(s)\n", 0)
  if (m != MTNONE) ERROR("Illegal Modifier %d in Constant Definition", m, EXIT_FAILURE)
  do {
	expect('#');  //Require # prefix
    getwrd();     //Get constant name
    DEBUG("Defining constant '%s',", word)
    strncpy(defnam[defcnt], word, VARLEN);
    setlbl(word); //Set label Assembler Line
    expect('=');
    defval[defcnt++] = prsbyt(); //Get Value
    ACMNT(word); //comment value
    asmlin(EQUOP, value); //Write Definition
    DETAIL(" defined as '%s'\n", value)
  } while (look(','));
  expect(';');
  DEBUG("Constant Declaration Completed\n", 0)
}

/* Parse Enum Declaration*/
void penum(int m) {
  int enmval = 0;
  DEBUG("Processing Enum Declarations\n", 0)
  if (m != MTNONE) ERROR("Illegal Modifier %d in Enum Definition", m, EXIT_FAILURE)
  expect('{');
  do {
    getwrd(); //get defined identifier
    DEBUG("Enumerating '%s'\n", word)
    strncpy(defnam[defcnt], word, VARLEN);
    setlbl(word); //Set label Assembler Line
    defval[defcnt++] = enmval; //Set Value
    sprintf(value, "%d", enmval);
    asmlin(EQUOP, value); //Write Definition
    DEBUG("Defined as '%s'\n", value)
    enmval++;
  } while (look(','));
  expect('}');
  expect(';');
  DEBUG("Enum Declaration Completed\n", 0)
}


/* Parse Variable/Function Declaration*/
void pdecl(int m, int t) {
  DEBUG("Processing variable declarations(s) of type %d\n", t)
  do {
    getwrd();
    if (match('(')) {
      if (m != MTNONE) ERROR("Illegal Modifier %d in Function Definition", m, EXIT_FAILURE)
      addfnc();  //Add Function Call
      return;
    }  
    addvar(m, t);
  } while (look(','));
  expect(';');
  DEBUG("Variable Declaration Completed\n", 0)
  SCMNT("");  //Clear Assembler Comment
}

/* Check for and Parse Type Keyword */
int ptype(int m) {
  int result = TRUE;
  if     (wordis("CONST")) pconst(m);        //Parse 'const' declaration
  else if (wordis("ENUM")) penum(m);         //Parse 'enum' declaration
  else if (wordis("CHAR")) pdecl(m, VTCHAR); //Parse 'char' declaration
  else if (wordis("VOID")) pdecl(m, VTVOID); //Parse 'void' declaration
  else                     result = FALSE;
  return result;
}

/* Check for and Parse Modifier */
int pmodfr(void) {
  DEBUG("Parsing modifier '%s'\n", word)
  int result = TRUE;
  if      (wordis("ALIGNED"))  { getwrd(); ptype(MTALGN); }
  else if (wordis("ZEROPAGE")) { getwrd(); ptype(MTZP); }
  else                         result = FALSE;
  return result;
}
