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

/* Push Term and Operator onto Stack */
void pshtrm(void) {
  if (trmidx >= MAXTRM) ERROR("Maximum Function Call/Array Index Depth Exceeded", 0, EXIT_FAILURE)
  oprstk[trmidx] = oper;        //Put Current Operator on Stack
  strcpy(trmstk[trmidx], term); //Put Current Term on Stack
  trmidx++;                     //Increment Stack Pointer
}

/* Pop Term and Operator off Stack */
void poptrm(void) {
  trmidx--;                     //Decrement Stack Pointer
  strcpy(term, trmstk[trmidx]); //Restore Current Term from Stack
  oper = oprstk[trmidx];        //Restore Current Operator from Stack
}

/* Parse value (literal or identifier)   *
 * Args: alwreg - allow registers        *
 8       alwcon - allow constants        *
 * Sets: value - the value (as a string) *
 *       valtyp - value type             */
void prsval(int alwreg, int alwcon) {
  DEBUG("Parsing value\n", 0)
  skpspc();
  if      (islpre()) prslit();       //Parse Literal
  else if (isalph()) prsvar(alwreg, alwcon); //Parse Variable
  else               expctd("literal or variable");
  DEBUG("Parsed value of type %d\n", valtyp)
  skpspc();
}

/* Process Unary Minus */
void prcmns(void) {
    DEBUG("Processing unary minus", 0)
    asmlin("LDA", "#$00");  //Handle Unary Minus
}

/* Parse array index                      *
 * Args: clbrkt - require closing bracket *
 * Sets: value - array index or           *
 *             "" if no index defined     */
void prsidx(int clbrkt) {
  expect('[');
  prsval(TRUE, TRUE); //Parse Value, Allowing Registers
  DEBUG("Parsed array index '%s'\n", value)
  if (clbrkt) expect(']');
}

/* Process Simple Array Index            *
 * Uses: term - array variable name      *
 *       valtyp - array index value type *
 *       value - array index as string   *
 *       word - array index raw string   *
 * Sets: term - modified variable name   */
void prcsix(void) {
    if (valtyp == LITERAL) {
      strcat(term, "+");
      strcat(term, word);
    }
    else if (strcmp(value, "Y")==0) 
      strcat(term, ",Y");
    else {
      if      (strcmp(value, "A")==0) asmlin("TAX", "");
      else if (strcmp(value, "X")!=0) asmlin("LDX", value);
      strcat(term, ",X");    
    }
}

/* Process Expression Array Index */
void prcxix(void) {
  pshtrm(); //Push Array Variable onto Term Stack
  if (trmcnt) asmlin("PHA", ""); //Save Accumulator if not first term
  prcftm(FALSE);    //Process First Term of Expression
  prsrxp(']'); //Parse Rest of Expression
  asmlin("TAX", ""); //Transfer Result of Expression to Index Register
  if (trmcnt) asmlin("PLA", ""); //Restore Accumator if not first term
  poptrm(); //Pop Array Variable off Term Stack
  strcat(term, ",X");
}

/* Check for, Parse, and Process Index */
void chkidx(void) {
  //DEBUG("Checking for Array Index with valtyp=%d\n", valtyp)
  if (valtyp == ARRAY) {
    if (look('-')) {
      prcmns();
	  prcxix();
    }
    else {
      prsidx(FALSE);
      if (valtyp > REGISTER) prcxix();
      else if (look(']')) prcsix();
      else prcxix();
    }
  }
}

/* Parse Term in Expression           *
 * Sets: term - the term (as a string) */
int prstrm(int alwint) {
  DEBUG("Parsing term\n", 0)
  prsval(FALSE, TRUE); //Parse Term - Disallow Registers
  if (valtyp == FUNCTION) ERROR("Function call only allowed in first term\n", 0, EXIT_FAILURE)
  strcpy(term, value);
  if (valtyp == VARIABLE && vartyp[varidx] == VTINT) {
    if (!alwint) ERROR("Illegal Use of Integer Variable %s\n", term, EXIT_FAILURE)
	prcvri(); //Process Integer Variable
	return TRUE;
  }
  DEBUG("Parsed term %s\n", term)
  chkidx();  //Check for Array Index
  skpspc();
  return FALSE;
}

/* Process Address Reference 
 * Args: adract = Address Action (adacts) * 
 *       symbol = Symbol to Process       */
void prcadr(int adract, char* symbol) {
  DEBUG("Processing address '%s'\n", word)
  strcpy(word,"#>(");
  strcat(word,symbol);
  strcat(word,")");
  if (adract == ADPUSH) { asmlin("LDA", word); asmlin("PHA", ""); }
  else asmlin("LDY", word);
  strcpy(word,"#<(");
  strcat(word,symbol);
  strcat(word,")");
  if (adract == ADPUSH) { asmlin("LDA", word); asmlin("PHA", ""); }
  else asmlin("LDX", word);
} 

/* Parse and Compile Address of Operator *
 * Args: adract = Address Action         */
void prsadr(int adract) {
  DEBUG("Parsing address\n", 0)
  if (isnpre()) prsnum(0xFFFF);
  else prsvar(FALSE, TRUE);
  if (adract) prcadr(adract, value);  //Compile Address Reference
  else strcpy(word, value); //Save for Calling Routine
}

/* Parse and Create Anonymous String *
 * Args: adract = Address Action     *
 *       alwstr = Allow String       */
void prsstr(int adract, int alwstr) {
  if (!alwstr) ERROR("Illegal String Reference", 0, EXIT_FAILURE)
  DEBUG("Parsing anonymous string\n", 0)
  newlbl(vrname);         //Generate Variable Name
  value[0] = 0;           //Use Variable Size 0
  setvar(MTNONE, VTCHAR); //Set Variable Name, Type, and Size
  prsdts();               //Parse Data String
  setdat();               //Set Variable Data
  varcnt++;               //Increment Variable Counter
  if (adract) prcadr(adract, vrname); //Compile Address Reference
  else strcpy(word, vrname); //Save for Calling Routine
}

/* Check for and Process Address or String *
 * Args: adract = Address Action           *
 *       alwstr = Allow String             */
int chkadr(int adract, int alwstr) {
  DEBUG("Checking for Address or String\n", 0)
  int result = TRUE;
  if (look('&')) prsadr(adract);
  else if (match('"')) prsstr(adract, alwstr);
  else result = FALSE;
  skpspc();
  return result;
}

/* Parse Function Parameters or Return Values */
void prsfpr(char trmntr) {
  if (!chkadr(ADLDYX, TRUE) && isxpre() || match('?')) {
    if (!look('?')) {if (prsxpf(0)) goto prsfne;}
    if (look(',') && !chkadr(ADLDYX, TRUE)) {
      if (!look('?')) {
		if (prstrm(TRUE)) goto prsfne;
        asmlin("LDY", term); 
	  }
      if (look(',')) { prsval(FALSE, TRUE); asmlin("LDX", value); }
    }
  }
  prsfne:
  expect(trmntr);
}

/* Parse function call */
void prsfnc(char trmntr) {
  DEBUG("Processing Function Call '%s'\n", term)
  //int argexp = FALSE; //Expression(s) in second and third argument
  pshtrm(); //Push Function Name onto Term Stack
  skpchr(); //skip open paren
  CCMNT('(');
  prsfpr(')'); //Parse Function Parameters
  expect(trmntr);
  poptrm(); //Pop Function Name off Term Stack
  asmlin("JSR", term);
  skpspc();
}

/* Process Integer Variable */
void prcvri(void) {
  DEBUG("Processing Integer Variable '%s'\n", value)
  asmlin("LDX", value);
  strcat(value, "+1");
  asmlin("LDY", value);
}

/* Process first term of expression */
int prcftm(int alwint) {
  DEBUG("Processing first term '%s'\n", value)
  strcpy(term, value);
  if (valtyp == VARIABLE && vartyp[varidx] == VTINT) {
    if (!alwint) ERROR("Illegal Use of Integer Variable %s\n", word, EXIT_FAILURE)
    prcvri();
	return TRUE;
  }
  if (valtyp == FUNCTION) prsfnc(0); //Parse Expression Function
  else if (wordis("A"))   return FALSE;
  else if (wordis("X"))   asmlin("TXA", "");
  else if (wordis("Y"))   asmlin("TYA", "");
  else                  { chkidx(); asmlin("LDA", term); }
  return FALSE;
}

/* Parse first term of expession            *
 * First term can include function calls    */
int prsftm(int alwint) {
  prsval(TRUE, TRUE); //Parse Value, Allowing Registers
  return prcftm(alwint);
}

/* Process Arithmetic or Bitwise Operator *
 *   and the term that follows it         */
void prcopr(void) {
  DEBUG("Processing operator '%c'\n", oper)
  switch(oper) {
    case '+': asmlin("CLC", ""); asmlin("ADC", term); break;  //Addition
    case '-': asmlin("SEC", ""); asmlin("SBC", term); break;  //Subtraction
    case '&': asmlin("AND", term); break;  //Bitwise AND
    case '!': //For systems that don't have pipe in character set
    case '|': asmlin("ORA", term); break;  //Bitwise OR
    case '^': asmlin("EOR", term); break;  //Bitwise XOR
    default:  ERROR("Unrecognized operator '%c'\n", oper, EXIT_FAILURE)
  }
  oper = 0;
}

/* Parse Remainder of Expression */
void prsrxp(char trmntr) {
  skpspc();
  while (isoper()) {
	trmcnt++; //Increment Expression Depth
    prsopr(); //Parse Operator
    prstrm(FALSE); //Parse Term
    prcopr(); //Process Operator
	trmcnt--; //Decrement Expression Depth
  } 
  expect(trmntr);
}

int prsxpp(char trmntr, int alwint) {
  DEBUG("Parsing expression\n", 0)
  skpspc();
  trmcnt = 0; //Initialize Expression Depth
  if (match('-')) prcmns(); //Process Unary Minus
  else if (prsftm(alwint)) return TRUE;  //Parse First Term
  prsrxp(trmntr); //Parse Remainder of Express
  return FALSE;
}

/* Parse and compile expression */
void prsxpr(char trmntr) {
  prsxpp(trmntr, FALSE);
}

/* Parse and compile function parameter expression *
 * Returns: TRUE if Integer Expression */
int prsxpf(char trmntr) {
  return prsxpp(trmntr, TRUE);
}

/* Parse and compile integer expression */
void prsxpi(char trmntr) {
  skpspc();
  if (!chkadr(TRUE, FALSE)) {
    if (isnpre()) {
    DEBUG("Parsing Integer Literal\n", 0) 
      int number = prsnum(0xFFFF);
      sprintf(value, "%d", number & 0xFF); asmlin("LDX", value);
      sprintf(value, "%d", number >> 8); asmlin("LDY", value);
    } else if (isalph()) {
      prsvar(FALSE, TRUE);
      if (valtyp == FUNCTION) {
        strcpy(term, value);
        prsfnc(0); //Parse Expression Function
      } else if (valtyp == VARIABLE && vartyp[varidx] == VTINT) {
        prcvri(); //Process Integer Variable
      } else {
        ERROR("Illegal Variable %s In Integer Expression", value, EXIT_FAILURE)
      }  
    } else {
      ERROR("Expected Integer Value or Function\n", 0, EXIT_FAILURE);
    }
  }
  expect(trmntr);
}
