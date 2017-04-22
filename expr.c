/***********************************
 * C02 Expression Parsing Routines *
 ***********************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include "common.h"
#include "asm.h"
#include "parse.h"
#include "vars.h"
#include "expr.h"

/* Parse value (constant or identifier)  *
 * Sets: value - the value (as a string) *
 *       valtyp - value type           */
void prsval()
{
  DEBUG("Parsing value\n", 0);
  //expdef();      //Check for and expand define -- BROKEN!
  if (iscpre())
    prscon(0xff); //Parse Constant
  else if (isalph())
    prsvar();     //Parse Variable
  else
    expctd("constant or variable");
  skpspc();
  ACMNT(word);
}

/* Parse array index                  *
 * Sets: value - array index or       *
 *             "" if no index defined */
void prsidx()
{
  expect('[');
  prsval();
  DEBUG("Parsed array index '%s'\n", value);
  expect(']');
}

/* Parse term in expression            *
 * Sets: term - the term (as a string) * 
 *       trmtxt - type of term      */
void prstrm()
{
  DEBUG("Parsing term\n", 0);
  prsval();
  if (valtyp == FUNCTION) {
    ERROR("Function call only allowed in first term\n", 0, EXIT_FAILURE);
  }
  strcpy(term, value);
  DEBUG("Parsed term %s\n", term);
  trmtxt = valtyp;
  if (trmtxt == ARRAY) {
    prsidx();
    asmlin("LDX", value);
    strcat(term, ",X");
  }
  skpspc();
}

/* Parse and Compile Pointer Dereference */
void prsadr(char dlmtr)
{
  prsvar();
  DEBUG("Dereferencing variable '%s'\n", value);
  ACMNT(value);
  strcpy(word,"#<");
  strcat(word,value);
  asmlin("LDX", word);
  strcpy(word,"#>");
  strcat(word,value);
  asmlin("LDY", word);
  if (dlmtr) expect(dlmtr);
}


/* Parse function call in first term of expression *
 * Function call may include term as an argument   */
void prsxfn() 
{
  char fnname[255]; /*Function name*/ 
  DEBUG("Processing expression function call '%s'...\n", term);
  strcpy(fnname, term);
  skpchr(); //skip open paren
  CCMNT('(');
  if (look('&'))
    prsadr(0);
  else if (isvpre()) {
    prstrm();
    asmlin("LDA", term);
  }
  expect(')');
  asmlin("JSR", fnname);
  skpspc();
}

/* Parse first term of expession            *
 * First term can include function calls    */
void prsftm()
{
  prsval();
  DEBUG("Processing first term '%s'...\n", value);
  strcpy(term, value);
  trmtxt = valtyp;
  if (trmtxt == FUNCTION) {
     prsxfn(); //Parse Expression Function
     return;
  }
  if (trmtxt == ARRAY) {
    prsidx();
    asmlin("LDX", value);
    strcat(term, ",X");
  }
  asmlin("LDA", term); 
}

/* Process Arithmetic or Bitwise Operator *
 *   and the term that follows it         */
void prcopr()
{
  DEBUG("Processing operator '%c'\n", oper);
  switch(oper)
  {
    case '+': 
      asmlin("CLC", "");
      asmlin("ADC", term);
      break;
    case '-':
      asmlin("SEC", "");
      asmlin("SBC", term);
      break;
    case '&':
      asmlin("AND", term);
      break;
    case '|':
      asmlin("ORA", term);
      break;
    case '^':
      asmlin("EOR", term);
      break;
    default:
      printf("Unrecognized operator '%c'\n", oper);
      exterr(EXIT_FAILURE);
  }
}

/* Parse and compile expression */
void prsxpr(char trmntr)
{
  DEBUG("Parsing expression\n", 0);
  skpspc();
  if (match('-')) 
    asmlin("LDA", "#$00");  //Handle Unary Minus
  else 
    prsftm(); //Parse First Term
  while (isoper())
  {
    prsopr(); //Parse Operator
    prstrm(); //Parse Term
    prcopr(); //Process Operator
  } 
  if (trmntr)
    expect(trmntr);
}

