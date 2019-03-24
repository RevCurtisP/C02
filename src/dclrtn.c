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

void addprm(char* prmtr) {
  reqvar(FALSE);          //Get Variable Name
  strcpy(prmtr, value);   //Copy to Parameter Variable
  prmcnt++;               //Increment # of Parameters
}

/* Add Function Definition */
void addfnc(void) {
  if (infunc) ERROR("Nested Function Definitions Not Allowed\n", 0, EXIT_FAILURE)
  expect('(');
  strcpy(fncnam, word);   //Save Function Name
  prmcnt = 0;             //Initialze Number of Parameters
  skpspc();               //Skip Spaces
  if (isalph()) {         //Parse Parameters
    addprm(prmtra);       //Get First Parameter
    if (look(',')) {      //Look for Comma
      addprm(prmtry);     //Get Second Parameter
      if (look(',')) {    //Look for Comma
        addprm(prmtrx);   //Get Third Parameter
      }
    }
  }
  expect(')');
  if (look(';')) return;    //Forward Definition
  infunc = TRUE;          //Set Inside Function Definition Flag
  DEBUG("Set infunc to %d\n", infunc)
  setlbl(fncnam);         //Set Function Entry Point
  if (prmcnt > 0) asmlin("STA", prmtra); //Store First Parameter
  if (prmcnt > 1) asmlin("STY", prmtry); //Store Second Parameter
  if (prmcnt > 2) asmlin("STX", prmtrx); //Store Third Parameter
  endlbl[0] = 0;          //Create Dummy End Label
  pshlbl(LTFUNC, endlbl); //and Push onto Stack
  bgnblk('{');           //Start Program Block
}

/* Add Constant to Contant Table *
 * Args: numval = value as int   *
 * Uses: defnam = constant name  *
 *       word = value as parsed  *
 *       value = value as number */
void addcon(int numval) {
    strncpy(connam[concnt], defnam, VARLEN);
    if (alcvar) setlbl(defnam); //Set label Assembler Line
    conval[concnt++] = numval; //Get Value
    if (alcvar) asmlin(EQUOP, value); //Write Definition
    DEBUG("Defined constant '%s'", defnam)
    DETAIL(" as '%s'\n", value)
    if (!alcvar) SCMNT(""); //Clear Comment
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
    strncpy(defnam, word, VARLEN);
    sprintf(value, "%d", enmval);
	addcon(enmval++);
  } while (look(','));
  expect('}');
  expect(';');
  DEBUG("Enum Declaration Completed\n", 0)
}

/* Parse Enum Declaration*/
void pstrct(int m) {
  DEBUG("Processing Struct Declarations\n", 0)
  getwrd(); //Parse Structure Name
  if (look('{')) defstc();  //Parse Struct Definition
  else           addstc();  //Parse and Compile Struct Declaration
  cmtlin();    //Write out declaration comment
}

/* Parse Variable/Function Declaration*/
void pdecl(int m, int t) {
  DEBUG("Processing declaration(s) of type %d\n", t)
  do {
    getwrd();
    if (match('(')) {
      if (m > MTNONE) ERROR("Illegal Modifier %d in Function Definition\n", m, EXIT_FAILURE)
      addfnc();  //Add Function Call
      return;
    }
    addvar(m, t);
  } while (look(','));
  expect(';');
  DEBUG("Declaration completed\n", 0)
  cmtlin();    //Write out declaration comment
}

/* Check for and Parse Type Keyword *
 * Args: m - Modifier Type          */
int ptype(int m) {
  int reslt = TRUE;
  if    (wordis("STRUCT")) pstrct(m);        //Parse 'const' declaration
  else if (wordis("ENUM")) penum(m);         //Parse 'enum' declaration
  else if (wordis("CHAR")) pdecl(m, VTCHAR); //Parse 'char' declaration
  else if (wordis("INT"))  pdecl(m, VTINT);  //Parse 'int' declaration
  else if (wordis("VOID")) pdecl(m, VTVOID); //Parse 'void' declaration
  else                     reslt = FALSE;
  return reslt;
}

int pmtype(int m) {
  getwrd();
  if (m == MTALGN && wordis("CONST")) {m = m | MTCONST; getwrd();}
  DEBUG("Parsing type %s\n", word)
  return ptype(m);
}

/* Check for and Parse Modifier */
int pmodfr(void) {
  DEBUG("Parsing modifier '%s'\n", word)
  int result = TRUE;
  if      (wordis("ALIAS"))    { pmtype(MTALS); }
  else if (wordis("ALIGNED"))  { pmtype(MTALGN); }
  else if (wordis("CONST"))    { pmtype(MTCONST); }
  else if (wordis("ZEROPAGE")) { pmtype(MTZP); }
  else                         result = FALSE;
  return result;
}
