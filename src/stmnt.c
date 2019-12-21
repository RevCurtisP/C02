/************************************
 * C02 Statement Compiling Routines *
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

/* Begin Program Block */
void bgnblk(char blkchr) {
  DEBUG("stmnt.bgnblk: Beginning program block\n", 0)
  if (blkchr) {
    DEBUG("stmnt.bgnblk: Expecting Block Delimiter '%c'\n", blkchr);
    expect(blkchr);
    inblck = TRUE;
  }
  else
    inblck = look('{');
  DEBUG("stmnt.bgnblk: Set INBLCK to %d\n", inblck)
  setblk(inblck);
}

/* End Program Block                        *
 * Args: blkflg: End of Multiline Block     */
void endblk(int blkflg) {
  int lbtype;
  DEBUG("stmnt.endblk: Ending program block with flag %d\n", blkflg)
  expect('}'); //Block End Character
  DEBUG("stmnt.endblk: Found inblck set to %d\n", inblck)
  if (inblck != blkflg) ERROR("Encountered '}' without matching '{'\n", 0, EXIT_FAILURE)
  lbtype = poplbl();
  if (lbtype == LTDO) pdowhl(); //Parse While at End of Do Loop
}

/* Parse Shortcut If */
void prssif(char trmntr) {
  newlbl(cndlbl);         //Create Label for "if FALSE" expression
  prscnd(')', FALSE);     //Parse Condition
  expect('?');          
  prsxpr(':');            //Parse "if TRUE" expression
  newlbl(skplbl);         //Create End of Expression Label
  asmlin("JMP", skplbl);  //Jump over "if FALSE" expression
  setlbl(cndlbl);         //Emit "if FALSE" label
  prsxpr(trmntr);         //Parse "if FALSE" expression
  setlbl(skplbl);         //Emit End of Expression Label
}

/* Process Array Index */
void prcidx(int idxtyp, char *name, char *index)
{
    if (strlen(index)) { 
      if (idxtyp == LITERAL) {
        strcat(name, "+");
        strcat(name, index);
      }
      else {
        asmlin("LDX", index);
        strcat(name,",X");
      }
    }
}

/* Set word to assignment variable *
 * adding write offset (if set)    */
void setasn(char *name, char ispntr) {
  if (ispntr) strcpy(word,"(");
  else word[0] = 0;
  strcat(word, name);
  if (wrtofs[0]) strcat(word, wrtofs);
  if (ispntr) strcat(word,")");
}

void prcixy(char rgstr, char* idx, int ivt) {
  DEBUG("stmnt.prcixy: Processing %c register indexed assignment\n", rgstr)
  if (strlen(idx)) { //Process X variable Index
    if (ivt != LITERAL) { 
     asmlin("PHA", "");  //Save Accumulator
     if (rgstr == 'X') asmlin("TXA", "");  //Transfer Return Value to Accumulator
     else asmlin("TYA", "");
     prcidx(ivt, word, idx); //Process Index
     asmlin("STA", word); //Store Return Value
     asmlin("PLA", ""); //Restore Accumulator
   } else {
      prcidx(ivt, word, idx); //Process Index
      if (rgstr == 'X') asmlin("STX", word); //Store Return Value
      else asmlin("STY", word);
   }
  }
}

/* Process Assignment of X and Y variables */
void prcaxy(void) {
  DEBUG("stmnt.prcaxy: Processing X assignment variable '%s'\n", xsnvar)
  if (xsnvar[0]) {
    setasn(xsnvar, FALSE);
    if (strlen(xsnidx)) prcixy('X', xsnidx, xsnivt); //Process Index
	else asmlin("STX", word); //Store Return Value
    xsnvar[0] = 0;
  }
  DEBUG("stmnt.axy: Processing Y assignment variable '%s'\n", ysnvar)
  if (ysnvar[0]) {
    setasn(ysnvar, FALSE);
    if (strlen(ysnidx)) prcixy('Y', ysnidx, ysnivt); //Process Index
    else asmlin("STY", word); //Store Return Value
    ysnvar[0] = 0;
  }
}

/* Process Assignment */
void prcasn(char trmntr, char ispntr) {
  expect('=');
  if (look('(')) prssif(trmntr); //Parse Shortcut If 
  else           prsxpr(trmntr); //Parse Expression
  prcaxy(); //Process X and Y assignments
  DEBUG("stmnt.prcasn: Checking if '%s' is a register\n", asnvar)
  if      (strcmp(asnvar, "X")==0) asmlin("TAX", "");
  else if (strcmp(asnvar, "Y")==0) asmlin("TAY", "");
  else if (strcmp(asnvar, "A")==0) return;
  DEBUG("stmnt.prcasn: Processing assignment variable '%s'\n", asnvar)
  setasn(asnvar, ispntr);
  if (asnidx[0]) { 
    if (ispntr) prcptx(asnidx); //Process Pointer Index
	else prcidx(asnivt, word, asnidx); //Process Index 
  }
  else if (ispntr && !cmos) {
    strcat(word, ",Y");
    asmlin("LDY","0");
  }
  asmlin("STA", word); //Store Return Value
}

/* Process Integer Assignment */
void prcasi(char trmntr) {
  DEBUG("stmnt.prcasi: Processing Integer Assignment\n", 0);
  if (look('[')) ERROR("Illegal use of Integer Variable %s as Array\n", vrname, EXIT_FAILURE)
  expect('=');
  strcpy(xsnvar, vrname); //Set Assignment LSB
  strcpy(ysnvar, vrname); strcat(ysnvar, "+1"); //Set Assignment MSB
  ysnidx[0] = 0; //No Y Index
  prsxpi(trmntr, TRUE); //Parse Integer Expression, Allowing Functions
  prcaxy();
}

/* Parse and Return Array Index and Type */
int getidx(char* idx) {
    prsidx(TRUE);  //Parse Array Index
    if (valtyp == LITERAL) strncpy(idx, word, VARLEN);
	else strncpy(idx, value, VARLEN);
    DEBUG("stmnt.getidx: Parsed index %s\n", idx)
    return valtyp;
}

/* Process Accumulator Assignment Variable */
int prcava(char *name, char trmntr, char ispntr) {
  strcpy(asnvar, name);
  asntyp = valtyp; //Set Assigned Variable Type
  DEBUG("stmnt.prcava: Set STA variable to %s\n", asnvar)
  if (asntyp == VARIABLE && look(';')) {
    asmlin("STA", asnvar);
    return TRUE;
  }
  if (asntyp == ARRAY) asnivt = getidx(asnidx); //Get Array Index and Type
  else asnidx[0] = 0;
  if (ispntr && strcmp(asnidx, "X") == 0) ERROR("Illegal use of register X\n", 0, EXIT_FAILURE) 
  DEBUG("stmnt.prcava: Set STA index to '%s'", asnidx) DETAIL(" and type to %d\n", asnivt)
  if (ispopr()) {
    if (prspst(trmntr, FALSE, asnvar, asnidx, asnivt, ispntr)) expctd("post operator");
	return TRUE;
  }
  return FALSE;
}

/* Process Assignment Variable(s) */
void prcavr(char trmntr) {
  chksym(TRUE, FALSE, word);
  DEBUG("stmnt.prcavr: Processing assignment of variable %s\n", word);
  strcpy(vrname, word);  //save variable to assign to
  if (valtyp == STRUCTURE) prsmbr(vrname); //Updates word and vartyp
  if (vartyp == VTINT) {
    if (ispopr()) {if (prspst(trmntr, TRUE, vrname, "", 0, FALSE)) expctd("post operator");}
    else prcasi(trmntr); //Process Integer Assignment
    return;
  }
  if (prcava(vrname, trmntr, FALSE)) return;
  if (look(',')) {
    if (asntyp == REGISTER) ERROR("Register %s not allowed in plural assignment\n", asnvar, EXIT_FAILURE)
    prsvar(FALSE, FALSE); //get variable name
    strcpy(ysnvar, word);
    DEBUG("stmnt.prcvar: Set STY variable to %s\n", ysnvar)
    if (valtyp == ARRAY) ysnivt = getidx(ysnidx); //Get Array Index and Type
    else ysnidx[0] = 0;
    DEBUG("stmnt.prcvar: Set STY index to '%s'", ysnidx) DETAIL(" and type to %d\n", ysnivt)
    if (look(',')) {
      prsvar(FALSE, FALSE); //get variable name
      strcpy(xsnvar, word);
      DEBUG("stmnt.prcvar: Set STX variable to %s\n", xsnvar)
      //if (valtyp == ARRAY) ERROR("Array element not allowed in third assignment\n", 0, EXIT_FAILURE) 
      if (valtyp == ARRAY) xsnivt = getidx(xsnidx); //Get Array Index and Type
      else xsnidx[0] = 0;
    }
  }
  prcasn(trmntr, FALSE);
}

/* Parse 'asm' String Parameter */
void pasmst(char trmntr) {
  skpspc(); //Skip Spaces
  if (!match('"')) expctd("string");
  getstr();
  skpspc();
  expect(trmntr);
}

/* Parse and Compile 'asm' statement */
void pasm(void) {
  char opcode[LINELEN];
  expect('(');
  pasmst(',');    
  if (strlen(word)) setlbl(word);
  pasmst(',');
  strcpy(opcode, word);
  pasmst(')');
  expect(';');
  asmlin(opcode, word);
}

/* Parse and Compile Assignment of Pointer */
void prcasp(char trmntr) {
  prsptr(); //Parse Pointer Dereference
  DEBUG("stmnt.prcasp: Processing assignment to dereferenced pointer %s\n", value)
  if (prcava(value, trmntr, TRUE)) return; //Process Accumulator Assignment Variable
  prcasn(trmntr, TRUE);
}

/* Parse and Compile an Assignment */
void prsasn(char trmntr) {
  if (match('*')) prcasp(trmntr);
  else {
    getwrd();               //Get Variable to be Assigned
    DEBUG("stmnt.prsasn: Parsing assignment of word %s\n", word)
    prcavr(trmntr);
  }
}

/* parse and compile 'break'/'continue' statement */
void pbrcnt(int lbflag) 
{
  DEBUG("stmnt.pbrcnt: Parsing BREAK/CONTINUE statement\n", 0)
  if (lstlbl(lbflag) < 0) ERROR("Break/continue statement outside of loop\n", 0, EXIT_FAILURE)
  DEBUG("stmnt.pbrcnt: Found Label '%s'\n", tmplbl)
  asmlin("JMP", tmplbl);
  expect(';'); 
}

/* parse and compile 'do' statement */
void pdo(void) {
  DEBUG("stmnt.pdo: Parsing DO statement '%c'\n", nxtchr)
  newlbl(endlbl);         //Create End Label
  pshlbl(LTDWHL, endlbl); //and Push onto Stack
  reqlbl(loplbl);         //Get or Create/Set Loop Label
  pshlbl(LTDO, loplbl);   //Push onto Stack
  bgnblk(FALSE);          //Check For and Begin Block
}

/* parse and compile 'while' after 'do' statement */
void pdowhl(void) {
  DEBUG("stmnt.dowhl: Parsing WHILE after DO '%c'\n", nxtchr)
  getwrd();               //Check for While
  if (!wordis("WHILE")) expctd("while statement");
  expect('(');
  strcpy(cndlbl, loplbl); //Set Conditional Label to Loop Label
  prscnd(')', TRUE);      //Parse Conditional Expession
  poplbl();               //Pop While Conditional Label
  setlbl(endlbl);         //and Set Label to Emit on Next Line
  expect(';');            //Check for End of Statement
}


/* parse and compile for statement */
void pfor(void) {
  DEBUG("stmnt.pfor: Parsing FOR statement '%c'\n", nxtchr);
  expect('(');
  prsasn(';');            //Process Initial Assignment     
  newlbl(forlbl);         //Create For Loop Conditional Label
  setlbl(forlbl);         //and Set to Emit on Next Line
  newlbl(endlbl);         //Create End Label
  pshlbl(LTEND, endlbl);  //and Push onto Stack
  newlbl(loplbl);         //Create Loop Label
  pshlbl(LTLOOP, loplbl); //and Push onto Stack
  newlbl(cndlbl);         //Create Conditional Label
  prscnd(';', TRUE);      //Parse Conditional Expession
  asmlin("JMP", endlbl);  //Jump over Increment
  setlbl(loplbl);         //Set to Emit on Next Line
  prsasn(')');            //Parse Increment Assignment
  asmlin("JMP", forlbl);  //Jump to Conditional
  setlbl(cndlbl);         //Emit Label at Start of Loop  
  bgnblk(FALSE);          //Check For and Begin Block
}

/* parse and compile if statement */
void pif(void) {
  DEBUG("stmnt.pif: Parsing IF statement\n", 0)
  expect('(');
  newlbl(cndlbl);      //Create New Label
  pshlbl(LTIF,cndlbl); //Push Onto Stack
  prscnd(')', FALSE);  //Parse Conditional Expession
  bgnblk(FALSE);       //Check For and Begin Block
}

/* parse and compile else statement */
void pelse(void) {
  DEBUG("stmnt.pelse: Parsing ELSE statement\n", 0)
  strcpy(lbltmp, lblasm);   //Save Line Label
  lblasm[0] = 0;            //and Clear It
  if (toplbl(skplbl) != LTELSE) { //Get Chained ELSE Label or
    newlbl(skplbl);               //Create Skip Label
    pshlbl(LTELSE, skplbl);       //Push Onto Stack
  }
  asmlin("JMP", skplbl);    //Emit Jump over Block Code
  strcpy(lblasm, lbltmp);   //Restore Line Label
  bgnblk(FALSE);            //Check For and Begin Block
}

/* parse and compile if statement */
void pgoto(void) {
  DEBUG("stmnt.pgoto: Parsing GOTO statement\n", 0)
  if (look('(')) {
    DEBUG("stmnt.pgoto: Processing Indirect GOTO\n", 0);
	prsxpi(')', FALSE); //Parse Integer Expression w/o Assembly
    sprintf(word, "(%s)", value); //Set Indirect Argument
  } else {
    if (!chkadr(ADNONE, FALSE)) getwrd();
  }
  expect(';');  
  asmlin("JMP", word);
}

/* parse and compile inline statement */
void pinlne(void) {
  DEBUG("stmnt.pinlne: Parsing INLINE statement\n", 0)
  do { 
    DEBUG("stmnt.pinlne: Parsing inline parameter\n", 0)
    if (look('&')) {
      reqvar(FALSE); //Get Variable Name
      strcpy(word, "<");
      strcat(word, value);
      strcat(word, ", >");
      strcat(word, value);
      asmlin(BYTEOP, word);
    }
    else if (look('"')) {
      value[0] = 0;
      while (!match('"')) {
        CCMNT(nxtchr);
        sprintf(word, "$%hhX,", getnxt());
        strcat(value, word);
      }
      strcat(value,"0");
      CCMNT(nxtchr);
      skpchr(); //Skip Terminating Quote
      asmlin(BYTEOP, value);
    }
    else {
      prslit(0xFF);  //Parse Literal
      sprintf(word, "$%hhX", litval); //not needed?
      asmlin(BYTEOP, value);
    }
  } while (look(','));
  expect(';');
}

/* parse and compile label */
void plabel(void) {
  prslab();      //Parse Program Label
  setlbl(word);  //Set Label to Emit
}

/* parse and compile pop statement */
void ppop(void) {
  DEBUG("stmnt.ppop: Parsing POP statement\n", 0)
  do {  
    if (look('.')) term[0]=0; 
    else {
      reqvar(TRUE);
      strcpy(term, value);
      chkidx();
    } 
    asmlin("PLA", "");     //Pop Value off Stack
    if (term[0]) asmlin("STA", term);
  } while (look(','));
  expect(';');  
}

/* parse and compile push statement */
void ppush(void) {
  DEBUG("stmnt.ppush: Parsing PUSH statement\n", 0)
  do { 
    if (!chkadr(ADPUSH, TRUE)) {
      prsxpr(0);        //Parse Expression
      asmlin("PHA",""); //Push Result on Stack
    }
  } while (look(','));
  expect(';');
}

/* parse and compile return statement */
void pretrn(int INTRPT) {
  DEBUG("stmnt.pretrn: Parsing RETURN statement\n", 0)
  skpspc();
  if (INTRPT) {
    expect(';');
    asmlin("RTI", "");
  } else {
    prsfpr(';'); //Parse Function Return Valuea
    asmlin("RTS", "");
  }
  lsrtrn = TRUE;  //Set RETURN flag
}

/* parse and compile select statement */
void pslct(void) {
  DEBUG("stmnt.pslct: Parsing SELECT statement\n", 0)
  expect('(');
  prsxpr(')');            //Parse Expression
  newlbl(endlbl);         //Create New Label
  pshlbl(LTSLCT,endlbl);  //Push Onto Stack
  bgnblk('{');            //Require Beginning of Block
  fcase = TRUE;
  strcpy(xstmnt, "CASE"); //Require Next Statement to be CASE
}

/* process end of case block */
void ecase(void) {
  DEBUG("stmnt.ecase: Processing end of CASE block\n", 0)
  if (poplbl(cndlbl) != LTCASE) ERROR("%s not at end of CASE block\n", word, EXIT_FAILURE)
  if (toplbl(endlbl) != LTSLCT) ERROR("Illegal nesting in SELECT statement\n", 0, EXIT_FAILURE)  
  asmlin("JMP", endlbl);  //Emit jump over default case
  setlbl(cndlbl);  //Set entry point label to emit
}

/* parse and compile select statement */
void pcase(void) {
  if (!fcase) ecase();     //Process end of case block
  skplbl[0] = 0;           //Clear Skip Label
  newlbl(cndlbl);          //Create Conditional Label
  pshlbl(LTCASE, cndlbl);  //and Push onto Stack
  while(TRUE) {
    prstrm(FALSE);         //Parse CASE argument
    if (!fcase || valtyp != LITERAL || litval) 
      asmlin("CMP", term); //Emit Comparison
    if (look(',')) {
      chklbl(skplbl);      //Emit skip to beginning of CASE block
      asmlin("BEQ", skplbl);
      fcase = 0;
	  continue;            //Parse next argument
    }
    bgnblk(':');           //Emit branch to end of CASE block
    asmlin("BNE", cndlbl); 
    break;
  }
  if (skplbl[0]) setlbl(skplbl); //Set CASE block label if defined
  fcase = FALSE;
}

/* parse and compile default statement */
void pdflt(void) {
  expect(':');
  ecase(); //Process end of case block
}

/* parse and compile while statement */
void pwhile(void) {
  DEBUG("stmnt.pwhile: Parsing WHILE statement '%c'\n", nxtchr)
  expect('(');
  newlbl(endlbl);          //Create End Label
  DEBUG("stmnt.pwhile: Created End Label %s, ", endlbl);
  pshlbl(LTEND, endlbl);   //and Push onto Stack
  reqlbl(loplbl);          //Get or Create/Set Loop Label
  DETAIL("Loop Label %s, ", loplbl);
  pshlbl(LTLOOP, loplbl);  //Push onto Stack
  if (!look(')')) {  
    newlbl(cndlbl);          //Create Conditional Skip Label
    DETAIL("Conditional Label %s, ", cndlbl);
    prscnd(')', TRUE);       //Parse Conditional Expession
    asmlin("JMP", endlbl);   //Emit Jump to End of Loop
    setlbl(cndlbl);          //and Set Label to Emit on Next Line
  }
  bgnblk(FALSE);           //Check For and Begin Block
}

/* generate unimplemented statement error */
void punimp(void) {
  ERROR("Unimplemented statement '%s' encountered\n", word, EXIT_FAILURE) 
}

/* Parse Function Call as Statement */
void prsfns(void) {
  strcpy(term, word);  //Copy Function Name
  prsfnc(';');            //Parse Function Call
  return;
}

/* parse and compile identifier (variable or function call) */
void prssym(void) {
  DEBUG("stmnt.prssym: Parsing Identifier %s\n", word)
  valtyp = gettyp();
  if (valtyp == FUNCTION) prsfns();  //Parse Statement Function Call
  else prcavr(';'); //Parse Assignment
}

/* parse and compile program statement */
void pstmnt(void) {
  DEBUG("stmnt.pstmnt: Parsing statement '%s'\n", word)
  //Keywords that start a new block
  if (wordis("DO"))      { pdo();    return; }
  if (wordis("ELSE"))    { pelse();  return; }
  if (wordis("FOR"))     { pfor();   return; }
  if (wordis("IF"))      { pif();    return; }
  if (wordis("SWITCH"))    ERROR("SWITCH not implemented. Use SELECT.\n", word, EXIT_FAILURE)
  if (wordis("SELECT"))  { pslct();  return; }
  if (wordis("CASE"))    { pcase();  return; }
  if (wordis("DEFAULT")) { pdflt();  return; }
  if (wordis("WHILE"))   { pwhile(); return; }
  if (match(':'))        { plabel(); return; }

  //Keywords that don't start a block
  if (wordis("ASM"))           pasm();
  else if (wordis("BREAK"))    pbrcnt(LFEND);
  else if (wordis("CONTINUE")) pbrcnt(LFBGN);
  else if (wordis("GOTO"))     pgoto();
  else if (wordis("INLINE"))   pinlne();
  else if (wordis("POP"))      ppop();
  else if (wordis("PUSH"))     ppush();
  else if (wordis("RESUME"))   pretrn(TRUE);
  else if (wordis("RETURN"))   pretrn(FALSE);
  else                         prssym();
  
  if (lblcnt && !inblck) {
    DEBUG("stmnt.pstmnt: Ending implied block\n", 0)
    if (poplbl() == LTDO) pdowhl(); //Parse While at End of Do Loop
  }
}
