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
  return e;
}

/* Process and Compile Comparison Operator and        *
 * Args: comparator - Encoded Comparison Operator     *
 * Uses: term - Term Being Compared Against           *
 *       label - Branch Target if Comparison is FALSE */
void prccmp(int cmprtr) 
{
  DEBUG("Processing comparison operator %d\n", cmprtr);
  switch(cmprtr) {
    case 1: // = or ==
      asmlin("CMP", term);
      asmlin("BNE", curlbl);
      break;
    case 2: // <
      if (cmpsgn) {
        asmlin("CMP", term);
        asmlin("BPL", curlbl);
      }
      else {
        asmlin("CMP", term);
        asmlin("BCS", curlbl);
      }
      break;
    case 3: // <= or =<
      if (cmpsgn) {
        asmlin("CMP", term);
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
      if (cmpsgn) {
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
      if (cmpsgn) {
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
    default:
      printf("Unsupported comparison operator index %d\n", cmprtr);
      exterr(EXIT_FAILURE);
  }
}

/* Parse Comparison Operator                      *
 * Sets: comparitor - Encoded Comparison Operator */
int prscmp() 
{
  cmpenc = enccmp(nxtchr);      //Encode Comparison Character
  if (cmpenc == 0) 
    expctd("comparison operator");
  cmprtr = cmpenc;              //Set Encoded Comparator 
  cmpenc = enccmp(nxtchr);      //Encode Next Comparison Character
  if (cmpenc != 0)
    cmprtr = cmprtr | cmpenc;   //Combine Encoded Comparator
  skpspc();
  DEBUG("Parsed comparator %d\n", cmprtr);
}

/* Parse and Compile Conditional Expression     *
 * Condition = <expression> <comparator> <term> */
void prscnd(char trmntr) 
{
  DEBUG("Parsing condition\n", 0);
  prsxpr(0);
  skpspc();
  prscmp();
  prstrm();
  CCMNT(trmntr);
  prccmp(cmprtr);
  expect(trmntr);
}

