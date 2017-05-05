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

/* Convert Comparison Operator Character *
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
  DEBUG("Expression signedness is %d\n", xprsgn);
  switch(cmprtr) {
    case 0: // Raw Expression (Skip)
      asmlin("BEQ", curlbl);
      break;
    case 1: // = or ==
      asmlin("CMP", term);
      asmlin("BNE", curlbl);
      break;
    case 2: // <
      if (xprsgn) {
        asmlin("CMP", term);
        asmlin("BPL", curlbl);
      }
      else {
        asmlin("CMP", term);
        asmlin("BCS", curlbl);
      }
      break;
    case 3: // <= or =<
      if (xprsgn) {
        asmlin("SEC", "");
        asmlin("SBC", term);
        asmlin("BVC", "+4");
        asmlin("EOR", "#$80");
        asmlin("BPL", curlbl);
        asmlin("BNE", curlbl);
      }
      else {
        asmlin("CLC", "");
        asmlin("SBC", term);
        asmlin("BCS", curlbl);
      }
      break;
    case 4: // >
      if (xprsgn) {
        asmlin("CMP", term);
        asmlin("BMI", curlbl);
        asmlin("BEQ", curlbl);
      }
      else {
        asmlin("CLC", "");
        asmlin("SBC", term);
        asmlin("BCC", curlbl);
      }
      break;
    case 5: // >= or =>
      if (xprsgn) {
        asmlin("CMP", term);
        asmlin("BMI", curlbl);
      }
      else {
        asmlin("CMP", term);
        asmlin("BCC", curlbl);
      }
      break;
    case 6: // <> or ><
      asmlin("CMP", term);
      asmlin("BEQ", curlbl);
      break;
    case 7: // Raw Expression (Normal)
      asmlin("BNE", curlbl);
      break;
    default:
      printf("Unsupported comparison operator index %d\n", cmprtr);
      exterr(EXIT_FAILURE);
  }
}

/* Parse Comparison Operator                      *
 * Sets: cmprtr - Encoded Comparison Operator */
int prscmp() 
{
  cmpenc = enccmp(nxtchr);      //Encode Comparison Character
  cmprtr = cmpenc;              //Set Encoded Comparator 
  if (cmprtr) {
    cmpenc = enccmp(nxtchr);    //Encode Next Comparison Character
    if (cmpenc != 0)
      cmprtr = cmprtr | cmpenc; //Combine Encoded Comparator
  }
  skpspc();
  DEBUG("Parsed comparator %d\n", cmprtr);
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
  prsxpr(0);
  skpspc();
  prscmp();
  if (cmprtr)
    prstrm();
  cmprtr = cmprtr ^ revrse & 7;
  prccmp();
  CCMNT(trmntr);
  expect(trmntr);
}

