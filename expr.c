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
#include "label.h"
#include "expr.h"

/* Set Expession Signedness */
setsgn(int sgndns)
{
  expsgn = sgndns;
  DEBUG("Set Expression Signedness to %d\n", expsgn);
}

/* Parse value (constant or identifier)  *
 * Sets: value - the value (as a string) *
 *       valtyp - value type           */
void prsval()
{
  DEBUG("Parsing value\n", 0);
  //expdef();      //Check for and expand define -- BROKEN!
  if (iscpre())
    prscon(0xff); //Parse Constant
  else if (isalph()) {
    prsvar();     //Parse Variable
    DEBUG("Checking type of variable '%s'\n", value);
    if (vartyp[lookup(value)] == VTBYTE) setsgn(TRUE);  
  }
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

/* Parse term in  expression            *
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

/* Compile Address Reference */
void prcadr(char* symbol)
{
  strcpy(word,"#<");
  strcat(word,symbol);
  asmlin("LDX", word);
  strcpy(word,"#>");
  strcat(word,symbol);
  asmlin("LDY", word);
} 

/* Parse and Compile Address of Operator */
void prsadr()
{
  prsvar();
  DEBUG("Parsing address of variable '%s'\n", value);
  ACMNT(value);
  prcadr(value);  //Compile Address Reference
}

/* Parse and Create Anonymous String */
void prsstr()
{
  DEBUG("Parsing anonymous string\n", 0);
  strcpy(tmplbl, curlbl);//Save Current Label
  newlbl();              //Generate Label Name
  strcpy(word, curlbl);  //and Use as Variable Name
  value[0] = 0;          //Use Variable Size 0
  setvar(VTCHAR);        //Set Variable Name, Type, and Size
  prsdts();              //Parse Data String
  setdat();              //Set Variable Data
  varcnt++;              //Increment Variable Counter
  prcadr(curlbl);        //Compile Address Reference
  strcpy(curlbl, tmplbl);//Restore Current Label
}

/* Parse Additional Function Parameters */
void prsfnp() 
{
  if (look(',')) {
    if (look('&'))
      prsadr();
    else if (match('"'))
      prsstr();
    else {
      prstrm();
      asmlin("LDY", term);
      if (look(',')) {
        prsval();
        asmlin("LDX", value);
      }  
    }
  }
}

/* Parse function call */
void prsfnc() 
{
  DEBUG("Processing Function Call '%s'...\n", term);
  if (fnscnt >= MAXFNS)
    ERROR("Maximum Function Call Depth Exceeded", 0, EXIT_FAILURE);
  strcpy(fnstck[fnscnt++], term);
  skpchr(); //skip open paren
  CCMNT('(');
  if (look('&'))
    prsadr();
  else if (match('"'))
    prsstr();
  else if (isvpre()) {
    prsxpr(0);
    prsfnp();
  }  
  expect(')');
  asmlin("JSR", fnstck[--fnscnt]);
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
     prsfnc(); //Parse Expression Function
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
  setsgn(FALSE); //Default Expression to Unsigned
  if (match('-')) {
    DEBUG("Processing unary minus", 0);
    asmlin("LDA", "#$00");  //Handle Unary Minus  
    setsgn(TRUE); //Expression is Signed
  } 
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

