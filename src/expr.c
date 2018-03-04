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

/* Parse value (constant or identifier)  *
 * Sets: value - the value (as a string) *
 *       valtyp - value type           */
void prsval(int alwreg) {
  DEBUG("Parsing value\n", 0)
  skpspc();
  if (iscpre())
    prscon(); //Parse Constant
  else if (isalph()) {
    prsvar(alwreg);     //Parse Variable
  }
  else
    expctd("constant or variable");
  DEBUG("Parsed value of type %d\n", valtyp)
  skpspc();
}

/* Parse array index                  *
 * Sets: value - array index or       *
 *             "" if no index defined */
void prsidx(void) {
  expect('[');
  prsval(TRUE);
  DEBUG("Parsed array index '%s'\n", value)
  expect(']');
}

/* Check for, Parse, and Process Index */
void chkidx(void) {
  //DEBUG("Checking for Array Index with valtyp=%d\n", valtyp)
  if (valtyp == ARRAY) {
    prsidx();
    if (valtyp == CONSTANT) {
      strcat(term, "+");
      strcat(term, word);
    }
    else if (strcmp(value, "Y")==0) 
      strcat(term, ",Y");
    else {
      if (strcmp(value, "A")==0)
        asmlin("TAX", "");
      else if (strcmp(value, "X")!=0)
        asmlin("LDX", value);
      strcat(term, ",X");    
    }
  }
}

/* Parse Term in Expression           *
 * Sets: term - the term (as a string) */
void prstrm(void) {
  DEBUG("Parsing term\n", 0)
  prsval(FALSE);
  if (valtyp == FUNCTION) 
    ERROR("Function call only allowed in first term\n", 0, EXIT_FAILURE)
  
  strcpy(term, value);
  DEBUG("Parsed term %s\n", term)
  chkidx();  //Check for Array Index
  skpspc();
}

/* Process Address Reference */
void prcadr(int adract, char* symbol) {
  DEBUG("Processing address '%s'\n", word)
  strcpy(word,"#>");
  strcat(word,symbol);
  if (adract == 1) {
    asmlin("LDA", word);
    asmlin("PHA", "");
  }
  else
    asmlin("LDY", word);
  strcpy(word,"#<");
  strcat(word,symbol);
  if (adract == 1) {
    asmlin("LDA", word);
    asmlin("PHA", "");
  }
  else
  asmlin("LDX", word);
} 

/* Parse and Compile Address of Operator */
void prsadr(int adract) {
  DEBUG("Parsing address\n", 0)
  if (isnpre()) 
    prsnum(0xFFFF);
  else
    prsvar(FALSE);
  prcadr(adract, value);  //Compile Address Reference
}

/* Parse and Create Anonymous String */
void prsstr(int adract) {
  DEBUG("Parsing anonymous string\n", 0)
  newlbl(vrname);         //Generate Variable Name
  value[0] = 0;           //Use Variable Size 0
  setvar(MTNONE, VTCHAR); //Set Variable Name, Type, and Size
  prsdts();               //Parse Data String
  setdat();               //Set Variable Data
  varcnt++;               //Increment Variable Counter
  prcadr(adract, vrname);  //Compile Address Reference
}

/* Check for and Process Address or String */
int chkadr(int adract) {
  DEBUG("Checking for Address or String\n", 0)
  int result = TRUE;
  if (look('&'))
    prsadr(adract);
  else if (match('"'))
    prsstr(adract);
  else
    result = FALSE;
  skpspc();
  return result;
}

/* Parse function call */
void prsfnc(char trmntr) {
  DEBUG("Processing Function Call '%s'\n", term)
  if (fnscnt >= MAXFNS)
    ERROR("Maximum Function Call Depth Exceeded", 0, EXIT_FAILURE)
  strcpy(fnstck[fnscnt++], term);
  skpchr(); //skip open paren
  CCMNT('(');
  if (!chkadr(0) && isxpre() || match('*')) {
    if (!look('*')) prsxpr(0);
    if (look(',')) {
      if (!chkadr(0)) {
        if (!look('*')) {
          prstrm();
          asmlin("LDY", term);
        }
        if (look(',')) {
          if (!look('*')) {
            prsval(FALSE);
            asmlin("LDX", value);
          }
        }
      }
    }
  }
  expect(')');
  expect(trmntr);
  asmlin("JSR", fnstck[--fnscnt]);
  skpspc();
}

/* Parse first term of expession            *
 * First term can include function calls    */
void prsftm(void) {
  prsval(TRUE);
  DEBUG("Processing first term '%s'\n", value)
  strcpy(term, value);
  if (valtyp == FUNCTION) {
     prsfnc(0); //Parse Expression Function
     return;
  }
  if (wordis("A"))
    return;
  if (wordis("X"))
    asmlin("TXA", "");
  else if (wordis("Y"))
    asmlin("TYA", "");
  else {
    chkidx(); //Check for Array Index
    asmlin("LDA", term); 
  }
}

/* Process Arithmetic or Bitwise Operator *
 *   and the term that follows it         */
void prcopr(void) {
  DEBUG("Processing operator '%c'\n", oper)
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
    case '!': //For systems that don't have pipe in character set
      asmlin("ORA", term);
      break;
    case '^': //Looks like an up-arrow in some character sets
      asmlin("EOR", term);
      break;
    default:
      ERROR("Unrecognized operator '%c'\n", oper, EXIT_FAILURE)
  }
  oper = 0;
}

/* Parse and compile expression */
void prsxpr(char trmntr) {
  DEBUG("Parsing expression\n", 0)
  skpspc();
  if (match('-')) {
    DEBUG("Processing unary minus", 0)
    asmlin("LDA", "#$00");  //Handle Unary Minus
  } 
  else 
    prsftm(); //Parse First Term
  skpspc();
  while (isoper())
  {
    prsopr(); //Parse Operator
    prstrm(); //Parse Term
    prcopr(); //Process Operator
  } 
  expect(trmntr);
}
