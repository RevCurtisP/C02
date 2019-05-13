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

int addprm(char* prmtr) {
  reqvar(FALSE);          //Get Variable Name
  if (vartyp == VTINT) {
    strcpy(prmtrx, value);
    strcpy(prmtry, value);
    strcat(prmtry, "+1");
    return TRUE;
  }
  strcpy(prmtr, value);   //Copy to Parameter Variable
  return FALSE;
}

/* Add Function Definition */
void addfnc(void) {
  if (infunc) ERROR("Nested Function Definitions Not Allowed\n", 0, EXIT_FAILURE)
  expect('(');
  strcpy(fncnam, word);   //Save Function Name
  prmtra[0] = 0;          //Initialze Parameters
  prmtry[0] = 0;
  prmtrx[0] = 0;
  skpspc();               //Skip Spaces
  if (isalph() || match('.')) {         //Parse Parameters
    if (!look('.')) {if (addprm(prmtra)) goto addfne;} //Get First Parameter
    if (look(',')) {      //Look for Comma
      if (addprm(prmtry)) goto addfne;     //Get Second Parameter
      if (look(',')) {    //Look for Comma
        addprm(prmtrx);   //Get Third Parameter
      }
    }
  }
  addfne:
  expect(')');
  if (look(';')) return;    //Forward Definition
  infunc = TRUE;          //Set Inside Function Definition Flag
  DEBUG("Set infunc to %d\n", infunc)
  setlbl(fncnam);         //Set Function Entry Point
  if (prmtra[0]) asmlin("STA", prmtra); //Store First Parameter
  if (prmtry[0]) asmlin("STY", prmtry); //Store Second Parameter
  if (prmtrx[0]) asmlin("STX", prmtrx); //Store Third Parameter
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

/* Parse Enum Declaration
*/
void penum(int m, int bitmsk) {
  int enmval = (bitmsk) ? 1 : 0;
  DEBUG("Processing Enum Declarations with BITMSK %d\n", bitmsk)
  if (m != MTNONE) ERROR("Illegal Modifier %d in Enum Definition", m, EXIT_FAILURE)
  expect('{');
  do {
    if (enmval > 0xFF) ERROR("Maximum ENUM or BITMASK value exceeded\n", 0, EXIT_FAILURE)
    if (look('.')) 
      DEBUG("Skipping sequence %d\n", enmval)
	else {
      getwrd(); //get defined identifier
      DEBUG("Enumerating '%s'", word)
      DEBUG(" as %d\n", enmval);
      strncpy(defnam, word, VARLEN);
      sprintf(value, "%d", enmval);
      addcon(enmval);
    }    
    if (bitmsk) enmval = enmval << 1;
    else enmval++;
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
 * Returns: Type (enum types)       */
int ctype(int reqtyp) {
  if     (wordis("BITMASK")) return TBITMASK;
  else if (wordis("STRUCT")) return TSTRUCT;
  else if   (wordis("ENUM")) return TENUM;
  else if   (wordis("CHAR")) return TCHAR;
  else if    (wordis("INT")) return TINT;
  else if   (wordis("VOID")) return TVOID;
  else if (reqtyp) ERROR("Type Declaration Expected\n", 0, EXIT_FAILURE)
  return TNONE;
}

/* Check for and Parse Type Keyword *
 * Args: m - Modifier Type          */
int ptype(int m) {
  int type = ctype(FALSE);
  switch (type) {
    case TSTRUCT:  pstrct(m); break;        //Parse 'struct' declaration
    case TENUM:    penum(m, FALSE); break;  //Parse 'enum' declaration
    case TBITMASK: penum(m, TRUE); break;   //Parse 'enum' declaration
    case TCHAR:    pdecl(m, VTCHAR); break; //Parse 'char' declaration
    case TINT:     pdecl(m, VTINT); break;  //Parse 'int' declaration
    case TVOID:    pdecl(m, VTVOID); break; //Parse 'void' declaration
    default:       return FALSE;
  }
  return TRUE;
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
