/************************************
 * C02 Conditional Parsing Routines *
 ************************************/
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
#include "label.h"
#include "cond.h"

int  cmprtr;    //Encoded Comparison Operator
int  cmpenc;    //Encoded Comparator Character

/* Encode Comparison Operator Character *
 * Args: Comparison Operator Character   *
 * Returns: Comparison Operator Bit Mask */
int enccmp(char c) 
{
  int e;
  DEBUG("Encoding Comparison Character '%c'\n", c);
  switch(c)
  {
    case '=': e = 1; break;
    case '<': e = 2; break;
    case '>': e = 4; break;
    default:  e = 0; 
  }
  if (e) {
    CCMNT(c);
    skpchr();
  }
  DEBUG("Encoded as %d\n", e);
  return e;
}

/* Process and Compile Comparison Operator and        *
 * Args: comparator - Encoded Comparison Operator     *
 * Uses: term - Term Being Compared Against           *
 *       label - Branch Target if Comparison is FALSE */
void prccmp() 
{
  DEBUG("Processing comparison operator %d\n", cmprtr);
  switch(cmprtr) {
    case 0: // Raw Expression (Skip)
      asmlin("BEQ", cndlbl);
      break;
    case 1: // = or ==
      asmlin("CMP", term);
      asmlin("BNE", cndlbl);
      break;
    case 2: // <
      asmlin("CMP", term);
      asmlin("BCS", cndlbl);
      break;
    case 3: // <= or =<
      asmlin("CLC", "");
      asmlin("SBC", term);
      asmlin("BCS", cndlbl);
      break;
    case 4: // >
      asmlin("CLC", "");
      asmlin("SBC", term);
      asmlin("BCC", cndlbl);
      break;
    case 5: // >= or =>
      asmlin("CMP", term);
      asmlin("BCC", cndlbl);
      break;
    case 6: // <> or ><
      asmlin("CMP", term);
      asmlin("BEQ", cndlbl);
      break;
    case 7: // Raw Expression (Normal)
      asmlin("BNE", cndlbl);
      break;
    default:
      fprintf(stderr, "Unsupported comparison operator index %d\n", cmprtr);
      exterr(EXIT_FAILURE);
  }
}

/* Parse Comparison */
int prscmp(int revrse) 
{
  skpspc();
  cmpenc = enccmp(nxtchr);      //Encode Comparison Character
  cmprtr = cmpenc;              //Set Encoded Comparator 
  if (cmprtr) {
    cmpenc = enccmp(nxtchr);    //Encode Next Comparison Character
    if (cmpenc != 0)
      cmprtr = cmprtr | cmpenc; //Combine Encoded Comparator
  }
  skpspc();
  if (cmprtr)
    prstrm();
  cmprtr = cmprtr ^ revrse & 7;
  prccmp();
  DEBUG("Parsed comparator %d\n", cmprtr);
}

/* Parse Flag Operator */
void prsflg(int revrse)
{
  DEBUG("Parsing Flag Operator '%c'\n", nxtchr);
  if (match('+')) 
    cmprtr = 0;
  else if (match('-'))
    cmprtr = 1;
  else
    expctd("Flag operator");
  skpchr();
  cmprtr = cmprtr ^ revrse & 1;
  if (cmprtr)
    asmlin("BPL", cndlbl);
  else
    asmlin("BMI", cndlbl);
}

/* Parse and Compile Conditional Expression     *
 * Condition = <expression> <comparator> <term> */
void prscnd(char trmntr, int revrse) 
{
  DEBUG("Parsing condition with revrse=%d\n", revrse);
  if (look('!')) {
    revrse = (revrse) ? FALSE: TRUE;
    DEBUG("Set revrse to %d\n", revrse);      
  }
  if (!look('*'))
    prsxpr(0);
  if (look(':'))
    prsflg(revrse);  //Parse Flag Operator
  else
    prscmp(revrse);  //Parse Comparison Operator
  expect(trmntr);
}
