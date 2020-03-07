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
  DEBUG("expr.pshtrm: Pushed term %s ", term)
  DETAIL("and operator '%onto stack'\n", oper)
}

/* Pop Term and Operator off Stack */
void poptrm(void) {
  trmidx--;                     //Decrement Stack Pointer
  strcpy(term, trmstk[trmidx]); //Restore Current Term from Stack
  oper = oprstk[trmidx];        //Restore Current Operator from Stack
  DEBUG("expr.pshtrm: Popped term %s ", term)
  DETAIL("and operator '%c' off stack\n", oper)
}

/* Parse value (literal or identifier)   *
 * Args: alwreg - allow registers        *
 8       alwcon - allow constants        *
 * Sets: value - the value (as a string) *
 *       valtyp - value type             */
void prsval(int alwreg, int alwcon) {
  DEBUG("expr.prsval: Parsing value\n", 0)
  skpspc();
  if      (islpre()) prslit();               //Parse Literal
  else if (isalph()) prsvar(alwreg, alwcon); //Parse Variable
  else if (isbtop()) prsbop();				 //Parse Byte Operator
  else               expctd("literal or variable");
  DEBUG("expr.prsval: Parsed value %s ", value)
  DETAIL("of type %d\n", valtyp)
  skpspc();
}

/* Process Unary Minus */
void prcmns(void) {
    DEBUG("expr.prcmns: Processing unary minus", 0)
    asmlin("LDA", "#$00");  //Handle Unary Minus
}

/* Parse array index                      *
 * Args: clbrkt - require closing bracket *
 * Sets: value - array index or           *
 *             "" if no index defined     */
void prsidx(int clbrkt) {
  expect('[');
  prsval(TRUE, TRUE); //Parse Value, Allow Registers & Constants
  DEBUG("expr.prsidx: Parsed array index '%s'\n", value)
  if (clbrkt) expect(']');
}

/* Process Simple Array Index            *
 * Uses: term - array variable name      *
 *       valtyp - array index value type *
 *       value - array index as string   *
 *       word - array index raw string   *
 * Sets: term - modified variable name   */
void prcsix(void) {
  DEBUG("expr.prcsix: Processing simple array index %s\n", word);
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
  DEBUG("expr.prcsix: Set term to %s\n", term);
}

/* Process Expression Array Index */
void prcxix(void) {
  DEBUG("expr.prcxix: Processing Expression Array Index", 0)
  pshtrm(); //Push Array Variable onto Term Stack
  if (trmcnt) asmlin("PHA", ""); //Save Accumulator if not first term
  prcftm(FALSE);    //Process First Term of Expression
  prsrxp(']'); //Parse Rest of Expression
  asmlin("TAX", ""); //Transfer Result of Expression to Index Register
  if (trmcnt) asmlin("PLA", ""); //Restore Accumator if not first term
  poptrm(); //Pop Array Variable off Term Stack
  strcat(term, ",X");
  DEBUG("expr.prcxix: Set term to %s\n", term);
}

/* Check for, Parse, and Process Index */
void chkidx(void) {
  //DEBUG("expr.chkidx: Checking for Array Index with valtyp=%d\n", valtyp)
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

/* Parse Pointer                 *
 * Sets: term - Compiled Pointer */
void prsptr(void) {
  DEBUG("expr.prsptr: Parsing pointer\n", 0)
  expect('*');  //Pointer Dereference Operator
  prsvar(FALSE,FALSE); //Parse Variable to Dereference
  strcpy(term, value);
  if (varble.modifr != MTZP) ERROR("Illegal dereference of non-pointer variable %s.\n", value, EXIT_FAILURE)
  DEBUG("expr.prsptr: Set term to %s\n", term);
}

/* Process Pointer Index         *
 * Sets: term - Compiled Pointer */
void prcptx(char *index) {
  DEBUG("expr.prcptx: Processing Dereferenced Pointer %s ", term)
  DETAIL("index [%s]\n", index)
  if (strcmp(index,"X")==0) ERROR("Illegal use of register X\n", 0, EXIT_FAILURE);
  if (strcmp(index,"A")==0) asmlin("TAY", "");
  else if (strcmp(index,"Y") != 0) asmlin("LDY", index);
}

/* Process Pointer                 *
 * Sets: term - Compiled Pointer */
int prcptr(void) {
  prsptr();
  DEBUG("expr.prcptr: Dereferencing Pointer %s\n", value);
  if (valtyp == ARRAY) {
    prsidx(TRUE);
	prcptx(value);
    sprintf(word, "(%s),Y", term);
  }  else if (cmos) {
    sprintf(word, "(%s)", term);
  } else {
	asmlin("LDY","0");
    sprintf(word, "(%s),Y", term);
  }
  strcpy(term, word);
  DEBUG("expr.prcptr: Set term to %s\n", term);
  return FALSE;  //Return Value Not an Integer
}

/* Parse Term in Expression            *
 * Sets: term - the term (as a string) *
 * Returns: TRUE if term is an integer */
int prstrm(int alwint) {
  DEBUG("expr.prstrm: Parsing term\n", 0)
  if (match('*')) return prcptr(); //Parse and Deference Pointer
  prsval(FALSE, TRUE); //Parse Value - Disallow Registers, Allow Constants
  if (valtyp == FUNCTION) ERROR("Function call only allowed in first term\n", 0, EXIT_FAILURE)
  strcpy(term, value);
  if (valtyp == VARIABLE && prcivr(alwint)) return TRUE;
  DEBUG("expr.prstrm: Parsed term %s\n", term)
  chkidx();  //Check for Array Index
  skpspc();
  return FALSE;
}

/* Process Address Reference 
 * Args: adract = Address Action (adacts) * 
 *       symbol = Symbol to Process       */
void prcadr(int adract, char* symbol) {
  DEBUG("expr.prcadr: Processing address '%s'\n", word)
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
  DEBUG("expr.prsadr: Parsing address\n", 0)
  if (isnpre()) prsnum(0xFFFF);
  else {
    getwrd();
    if (fndlab(word)) strcpy(value, word);
    else prsvrw(FALSE, TRUE);
  }
  if (adract) prcadr(adract, value);  //Compile Address Reference
  else strcpy(word, value); //Save for Calling Routine
}

/* Parse and Create Anonymous String *
 * Args: adract = Address Action     *
 *       alwstr = Allow String       */
void prsstr(int adract, int alwstr) {
  if (!alwstr) ERROR("Illegal String Reference", 0, EXIT_FAILURE)
  DEBUG("expr.prsstr: Parsing anonymous string\n", 0)
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
  DEBUG("expr.chkadr: Checking for Address or String\n", 0)
  int result = TRUE;
  if (look('&')) prsadr(adract);
  else if (match('"')) prsstr(adract, alwstr);
  else result = FALSE;
  skpspc();
  return result;
}

/* Parse Byte Operator */
void prsbop(void) {
  char byteop = getnxt();
  CCMNT(byteop);
  DEBUG("expr.prsbop: Parsing byte operator '%c'\n", byteop)
  if (chkadr(FALSE, FALSE)) {
    sprintf(value, "%c(%s)", byteop, word);
    valtyp = LITERAL;
  } else {
    reqvar(FALSE);
    if (vartyp != VTINT) ERROR("Integer Value Expected\n", 0, EXIT_FAILURE) 
    if (byteop == '>') strcat(value, "+1");
    vartyp = VTCHAR;
  }
  DEBUG("expr.prsbop: Set value to \"%s\"\n", value)
}

/* Parse Function Argument or Return Values */
void prsfpr(char trmntr) {
  int pusha = 0; int pushy = 0; //A and Y Arguments Pushed
  DEBUG("expr.prsfpr: Parsing Function Argument or Return Values\n", 0)
  if (!chkadr(ADLDYX, TRUE) && isxpre() || match('.')) {
    DEBUG("expr.prsfpr: Parsing Accumulator Expression\n", 0);
    if (look('.')) pusha = 255; 
    else {if (prsxpf(0)) goto prsfne;}
    if (look(',') && !chkadr(ADLDYX, TRUE)) {
      if (look('.')) {
        pushy = -1;
	  }
	  else {
	    if (look('(')) { 
		  if (pusha==0) {pusha = 1; asmlin("PHA","");} //Save A on Stack
          prsxpr(')'); asmlin("TAY", ""); //Evaluate Expression, and Copy to Y
		}
        else {
		  if (prstrm(TRUE)) goto prsfne;
          asmlin("LDY", term); 
	    }
	  } 
      if (look(',')) { 
	    if (look('(')) {
          if (pusha==0) {pusha = 1; asmlin("PHA","");} //Save A on Stack
          if (pushy==0) {pushy = 1; asmlin("PHA",""); asmlin("PHY","");} //Save Y on Stack
          prsxpr(')'); asmlin("TAX", ""); //Evaluate Expression, and Copy to X    
	    }
        else {
          prsval(FALSE, TRUE); //Parse Value - Disallow Registers, Allow Constants
          if (valtyp > VARIABLE) ERROR("Illegal Value Function Argument\n", 0, EXIT_FAILURE);
          if (valtyp == VARIABLE && vartyp != VTCHAR) ERROR("Illegal Variable Type\n", 0, EXIT_FAILURE);
          asmlin("LDX", value); 
        }
	  }
    }
  }
  prsfne:
  if (pushy==1) {asmlin("PLA",""); asmlin("TAY","");} //Pull Y Off Stack
  if (pusha==1) asmlin("PLA",""); //Pull A Off Stack 
  expect(trmntr);
}

/* Parse function call */
void prsfnc(char trmntr) {
  DEBUG("expr.prsfnc: Processing Function Call '%s'\n", term)
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
  DEBUG("expr.prcvri: Processing Integer Variable '%s'\n", value)
  asmlin("LDX", value);
  strcat(value, "+1");
  asmlin("LDY", value);
}

/* Process Integer Variable in Term                      *
 * Args: alwint = Allow Integer-Like Variable            *
 * Returns: Integer-Like Variable Processed - TRUE/FALSE */
int prcivr(int alwint) {
    switch (vartyp) {
      case VTINT:
        if (!alwint) ERROR("Illegal Use of Integer Variable %s\n", word, EXIT_FAILURE)
        prcvri();
	    return TRUE; 
	  case VTARRAY:
        if (!alwint) ERROR("Illegal Reference to Array %s\n", word, EXIT_FAILURE)
        prcadr(ADNONE, term);
	    return TRUE; 
      case VTSTRUCT:
        if (!alwint) ERROR("Illegal Reference to Struct %s\n", word, EXIT_FAILURE)
        prcadr(ADNONE, term);
	    return TRUE; 
	  default:
        return FALSE;
    }
}

/* Process first term of expression */
int prcftm(int alwint) {
  DEBUG("expr.prcftm: Processing first term '%s'\n", value)
  strcpy(term, value);
  if (valtyp == VARIABLE && prcivr(alwint)) return TRUE;
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
  DEBUG("expr.prsftm: Parsing first term\n", 0)
  if (match('*')) { 
    prcptr(); //Parse and Deference Pointer
    asmlin("LDA", term); 
    return FALSE;
  }
  prsval(TRUE, TRUE); //Parse Value, Allow Registers & Constants
  return prcftm(alwint);
}

/* Process Arithmetic or Bitwise Operator *
 *   and the term that follows it         */
void prcopr(void) {
  DEBUG("expr.prcopr: Processing operator '%c'\n", oper)
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
  DEBUG("expr.prsxpp: Parsing expression\n", 0)
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
  DEBUG("expr.prsxpf: Parsing Function Expression\n", 0)
  return prsxpp(trmntr, TRUE);
}

/* Parse and Compile Integer Expression *
 * (Address, Integer Literal, Variable, *
 *  Struct Member, or Function)         *
 * Args: trmntr - expression terminator *
 *       asmxpr - assemble expression   * 
 * Sets: value - Parsed Value or Symbol */
void prsxpi(char trmntr, int asmxpr) {
  skpspc();
  DEBUG("expr.prsxpi: Parsing integer expression\n", 0)
  if (!chkadr(TRUE, FALSE)) {
    if (isnpre()) {
    DEBUG("expr.prsxpi: Parsing Integer Literal\n", 0) 
      int number = prsnum(0xFFFF); //Parse Number into value
      if (asmxpr) {
        sprintf(value, "#%d", number & 0xFF); asmlin("LDX", value);
        sprintf(value, "#%d", number >> 8); asmlin("LDY", value);
      }
    } else if (isalph()) {
      prsvar(FALSE, TRUE);
      if (valtyp == FUNCTION) {
        strcpy(term, value);
        DEBUG("expr.prsxpi: Set term to %s\n", term)
        prsfnc(0); //Parse Expression Function
      } else if (valtyp == STRUCTURE) {
        prsmbr(value);
        if (vartyp != VTINT) ERROR("Illegal Member %s In Integer Expression", value, EXIT_FAILURE)        
      } else if (valtyp == VARIABLE && vartyp == VTINT) {
        if (asmxpr) prcvri(); //Process Integer Variable
      } else {
        ERROR("Illegal Variable %s In Integer Expression", value, EXIT_FAILURE)
      }  
    } else {
      ERROR("Expected Integer Value or Function\n", 0, EXIT_FAILURE);
    }
  }
  expect(trmntr);
}
