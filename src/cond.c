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
int enccmp(char c) {
  int e;
  DEBUG("Encoding Comparison Character '%c'", c)
  switch(c) {
    case '=': e = 1; break;
    case '<': e = 2; break;
    case '>': e = 4; break;
    default:  e = 0; 
  }
  if (e) { CCMNT(c); skpchr(); }
  DETAIL(", encoded as %d\n", e);
  return e;
}

/* Process and Compile Comparison Operator and        *
 * Args: comparator - Encoded Comparison Operator     *
 * Uses: term - Term Being Compared Against           *
 *       label - Branch Target if Comparison is FALSE */
void prccmp(void) {
  DEBUG("Processing comparator %d", cmprtr) DETAIL(" with REVCMP=%d\n", revcmp)
  if (cmprtr > 7) { //Process Flag
    cmprtr = (cmprtr ^ revcmp) & 1;  //Apply Reversal
    if (cmprtr) asmlin("BPL", cmplbl);
    else        asmlin("BMI", cmplbl);
    return;
  }
  cmprtr = (cmprtr ^ revcmp) & 7; //Apply reversal
  switch(cmprtr) {
    case 0: // Raw Expression (Skip)
      asmlin("BEQ", cmplbl); break;
    case 1: // = or ==
      asmlin("CMP", term); asmlin("BNE", cmplbl); break;
    case 2: // <
      asmlin("CMP", term); asmlin("BCS", cmplbl); break;
    case 3: // <= or =<
      asmlin("CLC", "");   asmlin("SBC", term);   asmlin("BCS", cmplbl); break;
    case 4: // >
      asmlin("CLC", "");   asmlin("SBC", term);   asmlin("BCC", cmplbl); break;
    case 5: // >= or =>
      asmlin("CMP", term); asmlin("BCC", cmplbl); break;
    case 6: // <> or ><
      asmlin("CMP", term); asmlin("BEQ", cmplbl); break;
    case 7: // Raw Expression (Normal)
      asmlin("BNE", cmplbl); break;
    default:
      ERROR("Unsupported comparison operator index %d\n", cmprtr, EXIT_FAILURE)
  }
}

/* Parse Comparison */
void prscmp(int revrse) {
  skpspc();
  cmpenc = enccmp(nxtchr);      //Encode Comparison Character
  cmprtr = cmpenc;              //Set Encoded Comparator 
  if (cmprtr) {
    cmpenc = enccmp(nxtchr);    //Encode Next Comparison Character
    if (cmpenc != 0) cmprtr = cmprtr | cmpenc; //Combine Encoded Comparator
  }
  skpspc();
  if (cmprtr) prstrm(FALSE);
  //prccmp(); - Do after check for logical operator
  DEBUG("Parsed comparator %d\n", cmprtr)
}

/* Parse Flag Operator */
void prsflg(int revrse) {
  DEBUG("Parsing Flag Operator '%c'\n", nxtchr)
  if (match('+'))      cmprtr = 8; //Bit 0 = 0
  else if (match('-')) cmprtr = 9; //Bit 1 = 1
  else                 expctd("Flag operator");
  skpchr();
}

/* Parse Logical Operator *
 * Sets: logops */ 
void prslop(void) {
  DEBUG("Checking for Logical Operator\n", 0)
  logopr = LOPNONE;
  skpspc();
  if (isalph()) {
    getwrd(); //Get Logical Operator      
    DEBUG("Parsing Logical Operator %s\n", word)
    if (wordis("AND")) logopr = LOPAND;
    else if (wordis("OR")) logopr = LOPOR;
    else ERROR("Encountered invalid token \"%s\"\n", word, EXIT_FAILURE)
  } 
  DEBUG("Set LOGOPR to %d\n", logopr)
}

/* Parse and Compile Conditional Expression     *
 * Condition = <expression> <comparator> <term> */
void prscnd(char trmntr, int revrse) {
  DEBUG("Parsing condition with REVRSE=%d\n", revrse)
  tmplbl[0] = 0;
  do {
    strcpy(cmplbl, cndlbl); DEBUG("Set CMPLBL to \"%s\"\n", cmplbl); 
    revcmp = revrse;
    if (look('!')) revcmp = (revcmp) ? FALSE: TRUE;
    DEBUG("Set REVCMP to %d\n", revcmp)
    if (!look('*')) prsxpr(0);
    if (look(':')) prsflg(revcmp);  //Parse Flag Operator
    else           prscmp(revcmp);  //Parse Comparison Operator
    prslop(); //Parse Logical Operator
    if (logopr == LOPOR) {   
      revcmp = (revcmp) ? FALSE: TRUE;
      DEBUG("Set REVCMP to %d\n", revcmp)
    }
    if (logopr && revcmp) {
      if (!tmplbl[0]) newlbl(tmplbl); 
      strcpy(cmplbl, tmplbl); DEBUG("Set CMPLBL to \"%s\"\n", cmplbl);
    }
    prccmp(); //Process Comparison/Flag Operator
  } while (logopr);
  if (tmplbl[0]) setlbl(tmplbl);
  expect(trmntr);
}
